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

#include "PythonHeaders.hh"
#include "util/bpenv.hh"
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
#define PATHDELIM ";"
#else
#define PATHDELIM ":"
#endif
#define PYTHONPATH "PYTHONPATH"

static void*
pythonThreadFunc(void* ctx) {
    std::string path((const char*)ctx);
    std::string pyPath = path + "/stdlib";
    std::string soPath = path + "/ext";
    std::string pyOldPythonPath = bp::env::getEnvVar(PYTHONPATH);
    std::string pyNewPythonPath = pyPath + PATHDELIM + soPath;
    bp::env::setEnvVar(PYTHONPATH, pyNewPythonPath);
    s_argv = (char**) calloc(2, sizeof(char*));
    s_argv[0] = "BrowserPlus Embedded Python";
    s_argv[1] = NULL;
    // Still unclear wether this is all right.  Probably performed in
    // a thread spawned at allocation time.
    Py_SetProgramName(s_argv[0]);
    Py_Initialize();
    PySys_SetArgv(s_argc, s_argv);
    bp_load_builtins();
    PyObject* modname = PyString_FromString("browserplus");
    PyObject* bpModule = PyImport_Import(modname);
    Py_XDECREF(modname);
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
                std::string pyExistingPythonPath = bp::env::getEnvVar(PYTHONPATH);
                std::string pyUpdatedPythonPath = pyExistingPythonPath + PATHDELIM + serviceDir;
                bp::env::setEnvVar(PYTHONPATH, pyUpdatedPythonPath);
                // Read python source file.
                std::string source = file::readFile(work->sarg);
                if (source.empty()) {
                    work->m_error = true;
                    work->m_verboseError.append("couldn't read: '" + work->sarg + "'");
                } else {
                    int error = PyRun_SimpleString(source.c_str());
                    if (error != 0) {
                        work->m_error = true;
                        work->m_verboseError = python::getLastError();
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
                }
            }
            else if (work->m_type == python::Work::T_AllocateInstance) {
                PyObject *m = PyImport_AddModule("__main__");
                PyObject *klass = PyObject_GetAttrString(m, python::BP_GLOBAL_DEF_SYM);
                // Initialize arguments.
                PyObject* initArgs = (PyObject*)bpObjectToPython(work->m_obj, 0);
                // NEEDSWORK!!!  Do we need to check if __init__ takes args and pass NULL if not?
                work->m_instance = PyObject_CallObject(klass, initArgs);
                if (work->m_instance == NULL) {
                    work->m_error = true;
                    work->m_verboseError = python::getLastError();
                }
                else {
                    gcArray.Register(work->m_instance);
                }
                Py_DECREF(initArgs);
                Py_DECREF(klass);
            }
            else if (work->m_type == python::Work::T_InvokeMethod) {
                int error = 0;
#if 0
                PyObject* tid = rb_uint_new(work->m_tid);
                PyObject* trans = rb_class_new_instance(1, &tid, bp_py_cTransaction);
#endif // 0
                g_bpCoreFunctions->log(BP_DEBUG, "executing func '%s'", work->sarg.c_str());
#if 0
                python::invokeFunction(work->m_instance, work->sarg.c_str(), &error, 2, trans, bpObjectToPython(work->m_obj, work->m_tid));
#endif // 0
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
                Py_DECREF(work->m_instance);
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
    bp::env::setEnvVar(PYTHONPATH, pyOldPythonPath);
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
