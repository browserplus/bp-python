/**
 * Copyright 2010, Yahoo!
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. Neither the name of Yahoo! nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "PythonHeaders.hh" // must be included before *any* other headers
#include "util/bpthread.hh"
#include "util/bpsync.hh"
#include "util/fileutils.hh"
#include "PythonInterpreter.hh"
#include "PythonWork.hh"
#include "PythonUtils.hh"
#include "Definition.hh"
#include "DataMapping.hh"
#include "BuiltIns.hh"
#include "ServiceGlobals.hh"
#include <list>

static char** s_argv;
static int s_argc = 1;

// The thread upon which the python interpreter will run.
static bp::thread::Thread s_pythonThread;
static bp::sync::Mutex s_pythonLock;
static bp::sync::Condition s_pythonCond;
static bool s_running = false;
static std::list<python::Work *> s_workQueue;

#ifdef WIN32
#define PATHDELIM "\\"
#define PATHSEP ";"
#else
#define PATHDELIM "/"
#define PATHSEP ":"
#endif
static const std::string PYTHONPATH_CONST("PYTHONPATH");

void
AppendPythonPath_BeforeInit(const std::string& s) {
    char* envVal = getenv(PYTHONPATH_CONST.c_str());
    std::string pyOldPythonPath = envVal != NULL ? envVal : "";
    std::string pyNewPythonPath = pyOldPythonPath + PATHSEP + s;
    std::string envValue = PYTHONPATH_CONST + "=" + pyNewPythonPath;
    putenv((char*)envValue.c_str());
}

void
AppendPythonPath_AfterInit(const std::string& s) {
    PyObject* m = PyImport_ImportModule("sys");
    if (m != NULL) {
        PyObject *o = PyObject_GetAttrString(m, "path");
        if (PyList_Check(o)) {
            PyList_Append(o, PyString_FromString(s.c_str()));
        }
    }
    Py_XDECREF(m);
}

static void*
pythonThreadFunc(void* ctx) {
#ifdef WIN32
    std::string ctxPath((const char*)ctx);
    std::string soPath1 = ctxPath + PATHDELIM + "lib";
    std::string soPath2 = soPath1 + PATHDELIM + "lib-tk";
    std::string soPath3 = soPath1 + PATHDELIM + "plat-win";
    std::string soPath4 = soPath1 + PATHDELIM + "site-packages";
    AppendPythonPath_BeforeInit(ctxPath);
    AppendPythonPath_BeforeInit(soPath1);
    AppendPythonPath_BeforeInit(soPath2);
    AppendPythonPath_BeforeInit(soPath3);
    AppendPythonPath_BeforeInit(soPath4);
#else // WIN32
    std::string ctxPath((const char*)ctx);
    std::string soPath1 = ctxPath + PATHDELIM + "lib";
    std::string soPath2 = soPath1 + PATHDELIM + "lib-old";
    std::string soPath3 = soPath1 + PATHDELIM + "lib-tk";
    std::string soPath4 = soPath1 + PATHDELIM + "plat-darwin";
    std::string soPath5 = soPath1 + PATHDELIM + "plat-mac";
    std::string soPath6 = soPath1 + PATHDELIM + "plat-mac" + "lib-scriptpackages";
    std::string soPath7 = soPath1 + PATHDELIM + "site-packages";
    AppendPythonPath_BeforeInit(ctxPath);
    AppendPythonPath_BeforeInit(soPath1);
    AppendPythonPath_BeforeInit(soPath2);
    AppendPythonPath_BeforeInit(soPath3);
    AppendPythonPath_BeforeInit(soPath4);
    AppendPythonPath_BeforeInit(soPath5);
    AppendPythonPath_BeforeInit(soPath6);
    AppendPythonPath_BeforeInit(soPath7);
#endif // WIN32
    s_argv = (char**)calloc(2, sizeof(char*));
    s_argv[0] = "BrowserPlus Embedded Python";
    s_argv[1] = NULL;
    // Still unclear wether this is all right.  Probably performed in
    // a thread spawned at allocation time.
    Py_SetProgramName(s_argv[0]);
    Py_Initialize();
    PySys_SetArgv(s_argc, s_argv);
    bp_load_builtins();
#ifdef WIN32
    AppendPythonPath_AfterInit(ctxPath);
    AppendPythonPath_AfterInit(soPath1);
    AppendPythonPath_AfterInit(soPath2);
    AppendPythonPath_AfterInit(soPath3);
    AppendPythonPath_AfterInit(soPath4);
#else // WIN32
    AppendPythonPath_AfterInit(ctxPath);
    AppendPythonPath_AfterInit(soPath1);
    AppendPythonPath_AfterInit(soPath2);
    AppendPythonPath_AfterInit(soPath3);
    AppendPythonPath_AfterInit(soPath4);
    AppendPythonPath_AfterInit(soPath5);
    AppendPythonPath_AfterInit(soPath6);
    AppendPythonPath_AfterInit(soPath7);
#endif // WIN32
    PyObject* bpModule = PyImport_ImportModule("browserplus_internal");
    // Let's release the spawning thread.
    s_pythonLock.lock();
    s_running = true;
    s_pythonCond.signal();
    s_pythonLock.unlock();
    // Allocate a stack based container for managing anonymous object lifetime.
    python::GCArray gcArray;
    // Now we'll block and wait for work.
    while (s_running) {
        // Pop an item off the queue and process it,
        // outside of the global data structure lock.
        python::Work* work = NULL;
        {
            s_pythonLock.lock();
            if (s_workQueue.size() > 0) {
                work = *(s_workQueue.begin());
                s_workQueue.erase(s_workQueue.begin());
            }
            s_pythonLock.unlock();
        }
        if (work != NULL) {
            if (work->m_type == python::Work::T_LoadService) {
                // First lets update require path.
                std::string serviceDir = file::dirname(work->sarg);
                AppendPythonPath_AfterInit(serviceDir);
                // Read python source file.
                std::string source = file::basefilename(work->sarg);
                PyObject* result = PyImport_ImportModule(source.c_str());
                if (result == NULL && result == Py_None) {
                    work->m_error = true;
                    PyObject *resultString = PyObject_Str(result);
                    Py_XDECREF(resultString);
                    work->m_verboseError = PyString_AsString(resultString);
                }
                else {
                    // Now it's time to pull out the global symbol
                    // BrowserPlusEntryPointClass
                    // and call its to_service_description method
                    // and we'll get a python data structure we can
                    // traverse to discover the python interface
                    work->m_desc = python::extractDefinition(work->m_verboseError);
                    if (work->m_desc == NULL) {
                        work->m_error = true;
                    }
                }
                Py_XDECREF(result);
            }
            else if (work->m_type == python::Work::T_AllocateInstance) {
                PyObject *m = PyImport_AddModule("browserplus");
                PyObject *klass = PyObject_GetAttrString(m, python::BP_GLOBAL_DEF_SYM);
				// NEEDSWORK!!!  Do we need to check if __init__ takes args and pass NULL if not?
				work->m_instance = PyObject_CallObject(klass, NULL);
				if (work->m_instance == NULL) {
					work->m_error = true;
					work->m_verboseError = python::getLastError();
				}
				else {
					gcArray.Register(work->m_instance);
				}
                Py_XDECREF(klass);
            }
            else if (work->m_type == python::Work::T_InvokeMethod) {
                int error = 0;
                PyObject* args = Py_BuildValue("l", work->m_tid);
                PyObject* kwds = Py_BuildValue("");
                // NEEDSWORK!!! Is there a way to allocate and initialize a PyTypeObject instance in one call??
                PyObject* trans = PyType_GenericNew((PyTypeObject*)bp_py_cTransaction, args, kwds);
                python::invokeMethod(trans, "__init__", &error, 1, PyLong_FromLong(work->m_tid));
                Py_XDECREF(kwds);
                Py_XDECREF(args);
                g_bpCoreFunctions->log(BP_DEBUG, "executing func '%s'", work->sarg.c_str());
                python::invokeMethod(work->m_instance, work->sarg.c_str(), &error, 2, trans, bpObjectToPython(work->m_obj, work->m_tid));
                if (error) {
                    g_bpCoreFunctions->postError(work->m_tid, "python.evalError", python::getLastError().c_str());
                }
                if (work->m_obj) {
                    delete work->m_obj;
                    work->m_obj = NULL;
                }
            }
            else if (work->m_type == python::Work::T_ReleaseInstance) {
                int error = 0;
                (void)python::invokeFunction(work->m_instance, "destroy", &error, 0);
                gcArray.Unregister(work->m_instance);
                Py_XDECREF(work->m_instance);
            }
            // Presence of syncLock indicates synchronous operation
            // (client freed).
            if (work->m_syncLock != NULL) {
                work->m_syncLock->lock();
                work->m_done = true;
                work->m_syncCond->signal();
                work->m_syncLock->unlock();
            } else {
                delete work;
            }
        }
    }
    Py_XDECREF(bpModule);
    // Now we'll block and wait for work.
    s_pythonLock.unlock();
    return NULL;
}

// A utility routine to synchronously run "work" on the python interpreter
// thread, blocking until the work is complete.
static void
runWorkSync(python::Work* work) {
    // *NOTE* In this case work will not be deleted by this function nor
    // the interpreter thread.
    s_pythonLock.lock();
    s_workQueue.push_back(work);
    s_pythonCond.signal();
    work->m_syncLock = new bp::sync::Mutex;
    work->m_syncCond = new bp::sync::Condition;
    work->m_syncLock->lock();
    s_pythonLock.unlock();
    do {
        work->m_syncCond->wait(work->m_syncLock);
    } while (!work->m_done);
    work->m_syncLock->unlock();
}

// A utility routine to asynchronously run "work" on the python interpreter
// thread.  Work is dynamically allocated by the caller and will
// be freed by the interpreter thread.
static void runWorkASync(python::Work* work) {
    s_pythonLock.lock();
    s_workQueue.push_back(work);
    s_pythonCond.signal();
    s_pythonLock.unlock();
}

void
python::initialize(const std::string& path) {
    if (!s_running) {
        s_pythonLock.lock();
        if (s_pythonThread.run(pythonThreadFunc, (void*)path.c_str())) {
            while (!s_running) {
                s_pythonCond.wait(&s_pythonLock);
            }
        }
        s_pythonLock.unlock();
    }
}

void
python::shutdown(void) {
    // Stop the python thread!
    if (s_running) {
        s_pythonLock.lock();
        s_running = false;
        s_pythonCond.signal();
        s_pythonLock.unlock();
        s_pythonThread.join();
    }
    Py_Finalize();
}

bp::service::Description*
python::loadPythonService(const std::string& pathToPythonFile, std::string& oError) {
    python::Work work(python::Work::T_LoadService);
    work.sarg.append(pathToPythonFile);
    runWorkSync(&work);
    if (work.m_error) {
        oError = work.m_verboseError;
    }
    // If non-null, caller owns.
    return work.m_desc;
}

void*
python::allocateInstance(const bp::Map* context) {
    python::Work work(python::Work::T_AllocateInstance);
    work.m_obj = context;
    runWorkSync(&work);
    if (work.m_error) {
        g_bpCoreFunctions->log(BP_ERROR, "failed to allocate instance: %s", work.m_verboseError.c_str());
        return NULL;
    }
    // If non-null, caller owns.
    return (void*)work.m_instance;
}

void
python::invoke(void* instance, const char* funcName, unsigned int tid, bp::Map* arguments) {
    // Set up a dynamically allocated structure with information about the method invocation.
    python::Work* work = new python::Work(python::Work::T_InvokeMethod);
    work->m_instance = (PyObject*)instance;
    work->sarg.append(funcName);
    work->m_tid = tid;
    if (arguments) {
        work->m_obj = arguments->clone();
    }
    else {
        work->m_obj = NULL;
    }
    // Asynchronously run this work, not waiting around for the results.
    runWorkASync(work);
}

void
python::destroyInstance(void* instance) {
    python::Work work(python::Work::T_ReleaseInstance);
    work.m_instance = (PyObject*)instance;
    runWorkSync(&work);
}
