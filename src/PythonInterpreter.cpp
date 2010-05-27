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

#include "PythonHeaders.hh"

#include <list>

static char ** s_argv;
static int s_argc = 1;

// the thread upon which the python interpreter will run
static bp::thread::Thread s_pythonThread;
static bp::sync::Mutex s_pythonLock;
static bp::sync::Condition s_pythonCond;
static bool s_running = false;
static std::list<python::Work *> s_workQueue;

static void * pythonThreadFunc(void * ctx)
{
    std::string path((const char *) ctx);

    s_argv = (char **) calloc(2, sizeof(char *));
    s_argv[0] = "BrowserPlus Embedded Python";
    s_argv[1] = NULL;

    // still unclear wether this is all right.  probably performed in
    // a thread spawned at allocation time
    Py_SetProgramName(s_argv[0]);
    Py_Initialize();
    PySys_SetArgv(s_argc, s_argv);
#if 0
    python_init();
    int error = 0;
    python_script("BrowserPlus Embedded Python");
    std::string pyPath = path + "/stdlib";
    std::string soPath = path + "/ext";
    python_incpush(pyPath.c_str());
    python_incpush(soPath.c_str());

    bp_load_builtins();

    // include "browserplus.py" which cleans up the service authors
    // definition semantics a bit
    rb_require("browserplus.py");
    rb_require("pathname");

    // let's release the spawning thread
    s_pythonLock.lock();
    s_running = true;
    s_pythonCond.signal();
    s_pythonLock.unlock();

    // allocate a stack based container for managing anonymous
    // object lifetime
    python::GCArray gcArray;
#endif // 0

#if 0
    // now we'll block and wait for work
    while (s_running) {
        // pop an item off the queue and process it,
        // outside of the global data structure lock
        python::Work * work = NULL;

        {
            s_pythonLock.lock();
            if (s_workQueue.size() > 0) {
                work = *(s_workQueue.begin());
                s_workQueue.erase(s_workQueue.begin());
            }
            s_pythonLock.unlock();
        }

        if (work == NULL) {
            PyObject* sleepyTime = rb_float_new(0.05);
            // run the interpreter for a little bit to let background
            // threads continue
            // XXX: this absolutely sucks.  How do we kick this
            //      python thread out of sleep when new work arrives?
            //      signals?  something that we can do from another
            //      thread safely is needed here.
            rb_funcall(rb_mKernel, rb_intern("sleep"), 1, sleepyTime);
        } else {
            if (work->m_type == python::Work::T_LoadService) {
                // first lets update require path
                std::string serviceDir = file::dirname(work->sarg);
                python_incpush(serviceDir.c_str());

                // read python source file
                std::string source = file::readFile(work->sarg);

                if (source.empty()) {
                    work->m_error = true;
                    work->m_verboseError.append("couldn't read: '" +
                                                work->sarg + "'");
                } else {
                    int error = 0;
                    (void) rb_eval_string_protect(source.c_str(), &error);

                    if (error) {
                        work->m_error = true;
                        work->m_verboseError = python::getLastError();
                    } else {
                        // now it's time to pull out the global symbol
                        // BrowserPlusEntryPointClass
                        // and call its to_service_description method
                        // and we'll get a python data structure we can
                        // traverse to discover the python interface
                        work->m_desc =
                            python::extractDefinition(work->m_verboseError);
                        if (work->m_desc == NULL) {
                            work->m_error = true;
                        }
                    }
                }
            }
            else if (work->m_type == python::Work::T_AllocateInstance)
            {
                int error = 0;
                PyObject* klass = rb_gv_get(python::BP_GLOBAL_DEF_SYM);

                // initialize arguments
                PyObject* initArgs = bpObjectToPython(work->m_obj, 0);
                int takesArg = 0;
                ID initialize = rb_intern("initialize");
                if (rb_method_boundp(klass, initialize, 0))
                {
                    PyObject* initMeth =
                        python::invokeFunction(
                            klass, "instance_method", &error,
                            1, ID2SYM(initialize));

                    if (initMeth) {
                        PyObject* arity = python::invokeFunction(
                            initMeth, "arity", &error, 0);
                        if (NUM2INT(arity) >= 1) {
                            takesArg = 1;
                        }
                    }
                }

                work->m_instance =
                    python::invokeFunction(klass, "new", &error, takesArg,
                                         initArgs);

                if (error) {
                    work->m_error = true;
                    work->m_verboseError = python::getLastError();
                } else {
                    gcArray.Register(work->m_instance);
                }
            }
            else if (work->m_type == python::Work::T_InvokeMethod)
            {
                int error = 0;
                PyObject* tid = rb_uint_new(work->m_tid);
                PyObject* trans = rb_class_new_instance(1, &tid,
                                                    bp_py_cTransaction);

                g_bpCoreFunctions->log(
                    BP_DEBUG, "executing func '%s'",
                    work->sarg.c_str());

                python::invokeFunction(
                    work->m_instance, work->sarg.c_str(),
                    &error, 2, trans,
                    bpObjectToPython(work->m_obj, work->m_tid));

                if (error) {
                    g_bpCoreFunctions->postError(
                        work->m_tid, "python.evalError",
                        python::getLastError().c_str());
                }

                if (work->m_obj) {
                    delete work->m_obj;
                    work->m_obj = NULL;
                }
            } else if (work->m_type == python::Work::T_ReleaseInstance) {
                if (rb_method_boundp(CLASS_OF(work->m_instance),
                                     rb_intern("destroy"), 1))
                {
                    (void) python::invokeFunction(work->m_instance,
                                                "destroy", &error, 0);
                }

                gcArray.Unregister(work->m_instance);
            }

            // presence of syncLock indicates synchronous operation
            // (client freed)
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
#endif // 0

    // now we'll block and wait for work
    s_pythonLock.unlock();

    return NULL;
}

// a utility routine to synchronously run "work" on the python interpreter
// thread, blocking until the work is complete
static void runWorkSync(python::Work * work)
{
    // *note* in this case work will not be deleted by this function nor
    // the interpreter thread
    s_pythonLock.lock();
    s_workQueue.push_back(work);
    s_pythonCond.signal();
    work->m_syncLock = new bp::sync::Mutex;
    work->m_syncCond = new bp::sync::Condition;
    work->m_syncLock->lock();
    s_pythonLock.unlock();
    do { work->m_syncCond->wait(work->m_syncLock); } while (!work->m_done);
    work->m_syncLock->unlock();
}

// a utility routine to asynchronously run "work" on the python interpreter
// thread. work is dynamically allocated by the caller and will
// be freed by the interpreter thread
static void runWorkASync(python::Work * work)
{
    s_pythonLock.lock();
    s_workQueue.push_back(work);
    s_pythonCond.signal();
    s_pythonLock.unlock();
}

void python::initialize(const std::string & path)
{
    if (!s_running) {
        s_pythonLock.lock();
        if (s_pythonThread.run(pythonThreadFunc, (void *) path.c_str())) {
            while (!s_running) s_pythonCond.wait(&s_pythonLock);
        }
        s_pythonLock.unlock();
    }
}

void python::shutdown(void)
{
    // stop the python thread!
    if (s_running) {
        s_pythonLock.lock();
        s_running = false;
        s_pythonCond.signal();
        s_pythonLock.unlock();
        s_pythonThread.join();
    }
    Py_Finalize();
}

bp::service::Description *
python::loadPythonService(const std::string & pathToPythonFile,
                      std::string & oError)
{
#if 0
    python::Work work(python::Work::T_LoadService);
    work.sarg.append(pathToPythonFile);

    runWorkSync(&work);

    if (work.m_error) {
        oError = work.m_verboseError;
    }

    return work.m_desc; // if non-null, caller owns
#else // 0
    return NULL;
#endif // 0
}

void *
python::allocateInstance(const bp::Map * context)
{
#if 0
    python::Work work(python::Work::T_AllocateInstance);
    work.m_obj = context;

    runWorkSync(&work);

    if (work.m_error) {
        g_bpCoreFunctions->log(
            BP_ERROR,
            "failed to allocate instance: %s",
            work.m_verboseError.c_str());
        return NULL;
    }

    return (void *) work.m_instance; // if non-null, caller owns
#else // 0
    return NULL;
#endif // 0
}


void
python::invoke(void * instance, const char * funcName,
             unsigned int tid, bp::Map * arguments)
{
#if 0
    // set up a dynamically allocated structure with information about
    // the method invocation
    python::Work * work = new python::Work(python::Work::T_InvokeMethod);
    work->m_instance = (PyObject*) instance;
    work->sarg.append(funcName);
    work->m_tid = tid;
    if (arguments) { work->m_obj = arguments->clone(); }
    else work->m_obj = NULL;

    // asynchronously run this work, not waiting around for the results
    runWorkASync(work);
#endif // 0
}


void
python::destroyInstance(void * instance)
{
#if 0
    python::Work work(python::Work::T_ReleaseInstance);
    work.m_instance = (PyObject*) instance;
    runWorkSync(&work);
#endif // 0
}
