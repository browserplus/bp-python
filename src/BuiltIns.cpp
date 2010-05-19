#if 0
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
 */

#include "BuiltIns.hh"
#include "ServiceGlobals.hh"
#include "DataMapping.hh"
#include "PythonUtils.hh"

#include "util/bpsync.hh"

#include <assert.h>

#include "PythonHeaders.hh"

PyObject* bp_rb_cTransaction;
PyObject* bp_rb_cCallback;

static PyObject*
trans_init(PyObject* obj, PyObject* tid)
{
    (void) rb_ivar_set(obj, rb_intern("tid"), tid);
    return Qtrue;
}

static PyObject*
trans_tid(PyObject* obj)
{
    return rb_ivar_get(obj, rb_intern("tid"));
}

static PyObject*
trans_complete(int argc, PyObject* * argv, PyObject* obj)
{
    PyObject* tidVal = rb_ivar_get(obj, rb_intern("tid"));

    if (TYPE(tidVal) != T_FIXNUM && TYPE(tidVal) != T_BIGNUM) {
        rb_raise(rb_eException, "internal error, incorrect 'tid' ivar");
    } else if (argc > 1) {
        rb_raise(rb_eArgError, "wrong number of arguments");
    }
    
    unsigned int tid = NUM2UINT(tidVal);

    bp::Object * data = NULL;
    if (argc == 1) data = pythonToBPObject(argv[0]);
    const BPElement * e = (data == NULL) ? NULL : data->elemPtr();
    g_bpCoreFunctions->postResults(tid, e);
    
    if (data != NULL) delete data;

    return Qnil;
}

// keeping track of outstanding procs
static bp::sync::Mutex s_lock;
std::map<unsigned int, PyObject*> s_outstandingPrompts;

static void dummyCB(void * context, unsigned int promptId,
                    const BPElement * response)
{
    s_lock.lock();
    // figure out if we know about this prompt id
    std::map<unsigned int, PyObject*>::iterator it;
    it = s_outstandingPrompts.find(promptId);
    if (it != s_outstandingPrompts.end()) {
//         PythonInvokeProc(it->second, &(it->second),
//                        bp::Object::build(response));
        // happens sync
        s_outstandingPrompts.erase(it);
    }
    s_lock.unlock();
}

static PyObject*
trans_prompt(int argc, PyObject* * argv, PyObject* obj)
{
    PyObject* tidVal = rb_ivar_get(obj, rb_intern("tid"));

    if (TYPE(tidVal) != T_FIXNUM && TYPE(tidVal) != T_BIGNUM) {
        rb_raise(rb_eException, "internal error, incorrect 'tid' ivar");
    } else if (argc != 2) {
        rb_raise(rb_eArgError, "wrong number of arguments");
    } else if (!rb_block_given_p()) {
        rb_raise(rb_eArgError, "block required for prompt method");
    } else if (TYPE(argv[0]) != T_STRING) {
        rb_raise(rb_eException,
                 "transaction.prompt requires a string path argument");
    }
    
    unsigned int tid = NUM2UINT(tidVal);
    std::string path(RSTRING_PTR(argv[0]));
    bp::Object * data = pythonToBPObject(argv[1]);
    const BPElement * e = (data == NULL) ? NULL : data->elemPtr();

    s_lock.lock();
    unsigned int x = g_bpCoreFunctions->prompt(tid, path.c_str(), e,
                                               dummyCB, NULL);

    // grab the passed in block and increment a reference to it
    PyObject* val = rb_block_proc();
    s_outstandingPrompts[x] = val;
    rb_gc_register_address(&s_outstandingPrompts[x]);
    s_lock.unlock();

    // now call the block!
    
    if (data != NULL) delete data;

    // return the prompt id!
    return Qnil;
}

static PyObject*
trans_error(int argc, PyObject* * argv, PyObject* obj)
{
    PyObject* tidVal = rb_ivar_get(obj, rb_intern("tid"));
    const char * error = "unknownError";
    const char * verboseError = NULL;

    if (TYPE(tidVal) != T_FIXNUM && TYPE(tidVal) != T_BIGNUM) {
        rb_raise(rb_eException, "internal error, incorrect 'tid' ivar");
    } else if (argc > 2) {
        rb_raise(rb_eArgError, "wrong number of arguments");
    }
    
    unsigned int tid = NUM2UINT(tidVal);

    PyObject* errorVal = (argc > 0) ? argv[0] : 0;
    PyObject* verboseErrorVal = (argc > 1) ? argv[1] : 0;    

    if (errorVal) {
        if (TYPE(errorVal) != T_STRING) {
            rb_raise(rb_eException,
                     "bp_post_error called with non-string \"error\" argument");
        }
        error = RSTRING_PTR(errorVal);
    }

    if (TYPE(verboseErrorVal) == T_STRING) {
        verboseError = RSTRING_PTR(verboseErrorVal);
    }

    g_bpCoreFunctions->postError(tid, error, verboseError);

    return Qnil;
}

static PyObject*
callb_init(PyObject* obj, PyObject* tid, PyObject* cid)
{
    (void) rb_ivar_set(obj, rb_intern("tid"), tid);
    (void) rb_ivar_set(obj, rb_intern("cid"), cid);
    return Qtrue;
}

static PyObject*
callb_tid(PyObject* obj)
{
    return rb_ivar_get(obj, rb_intern("tid"));
}

static PyObject*
callb_cid(PyObject* obj)
{
    return rb_ivar_get(obj, rb_intern("cid"));
}

static PyObject*
callb_invoke(int argc, PyObject* * argv, PyObject* obj)
{
    if (argc > 1) rb_raise(rb_eArgError, "wrong number of arguments");

    unsigned int tid = 0;
    BPCallBack cid = 0;

    PyObject* tidVal = callb_tid(obj);
    PyObject* cidVal = callb_cid(obj);

    if (TYPE(tidVal) != T_FIXNUM && TYPE(tidVal) != T_BIGNUM) {
        rb_raise(rb_eException, "internal error, incorrect 'tid' ivar");
    }
    if (TYPE(cidVal) != T_FIXNUM && TYPE(cidVal) != T_BIGNUM) {
        rb_raise(rb_eException, "internal error, incorrect 'cid' ivar");
    }

    tid = NUM2UINT(tidVal);
    cid = NUM2UINT(cidVal);

    bp::Object * data = NULL;
    if (argc > 0) data = pythonToBPObject(argv[0]);

    const BPElement * e = (data == NULL) ? NULL : data->elemPtr();

    g_bpCoreFunctions->invoke(tid, cid, e);

    if (data != NULL) delete data;

    return Qnil;
}

void
bp_load_builtins()
{
    static bool initd = false;
    assert(!initd);
    initd = true;
    
    bp_rb_cTransaction = rb_define_class("BPTransaction", rb_cObject);
    rb_define_method(bp_rb_cTransaction, "initialize",
                     (PyObject* (*)(...)) trans_init, 1);
    rb_define_method(bp_rb_cTransaction, "complete",
                     (PyObject* (*)(...)) trans_complete, -1);
    rb_define_method(bp_rb_cTransaction, "prompt",
                     (PyObject* (*)(...)) trans_prompt, -1);
    rb_define_method(bp_rb_cTransaction, "error",
                     (PyObject* (*)(...)) trans_error, -1);
    rb_define_method(bp_rb_cTransaction, "tid",
                     (PyObject* (*)(...)) trans_tid, 0);
    
    bp_rb_cCallback = rb_define_class("BPCallback", rb_cObject);
    rb_define_method(bp_rb_cCallback, "initialize",
                     (PyObject* (*)(...)) callb_init, 2);
    rb_define_method(bp_rb_cCallback, "invoke",
                     (PyObject* (*)(...)) callb_invoke, -1);
    rb_define_method(bp_rb_cCallback, "tid",
                     (PyObject* (*)(...)) callb_tid, 0);
    rb_define_method(bp_rb_cCallback, "cid",
                     (PyObject* (*)(...)) callb_cid, 0);
}
#endif // 0
