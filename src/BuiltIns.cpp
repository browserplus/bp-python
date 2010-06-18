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

#include "PythonHeaders.hh" // must be included before *any* other headers
#include "BuiltIns.hh"
#include "PythonUtils.hh"
#include "DataMapping.hh"
#include "ServiceGlobals.hh"
#include "util/bpsync.hh"
#include "structmember.h"

void* /*PyObject**/ bp_py_cTransaction;
void* /*PyObject**/ bp_py_cCallback;

/*****************************/
/*** BEGIN TRANSACTION DEF ***/
/*****************************/
typedef struct {
    PyObject_HEAD
    unsigned int tid;
} Transaction;

static void
Transaction_dealloc(Transaction* self) {
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject*
Transaction_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Transaction* self = (Transaction*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->tid = 0;
    }
    return (PyObject*)self;
}

static int
Transaction_init(Transaction* self, PyObject* args, PyObject* kwds) {
    static char* kwlist[] = {"tid", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &self->tid)) {
        return -1;
    }
    return 0;
}

static PyMemberDef Transaction_members[] = {
    {"tid", T_LONG, offsetof(Transaction, tid), 0, "Transaction ID"},
    {NULL} /* Sentinel */
};

static PyObject*
trans_init(Transaction* self, PyObject* args) {
    unsigned int tid;
    if (!PyArg_ParseTuple(args, "I", &tid)) {
        return NULL;
    }
    self->tid = tid;
    Py_RETURN_NONE;
}

static PyObject*
trans_complete(Transaction* self, PyObject* args) {
    PyObject* pyData;
    if (!PyArg_ParseTuple(args, "O", &pyData)) {
        return NULL;
    }
    bp::Object* bpData = pythonToBPObject(pyData);
    const BPElement* e = (bpData == NULL) ? NULL : bpData->elemPtr();
    g_bpCoreFunctions->postResults(self->tid, e);
    if (bpData != NULL) {
        delete bpData;
    }
    Py_RETURN_NONE;
}

// Keeping track of outstanding procs.
static bp::sync::Mutex s_lock;
std::map<unsigned int, PyObject*> s_outstandingPrompts;

static void dummyCB(void* context, unsigned int promptId, const BPElement* response) {
    s_lock.lock();
    // Figure out if we know about this prompt id.
    std::map<unsigned int, PyObject*>::iterator i = s_outstandingPrompts.find(promptId);
    if (i != s_outstandingPrompts.end()) {
        //PythonInvokeProc(i->second, &(i->second), bp::Object::build(response));
        s_outstandingPrompts.erase(i);
    }
    s_lock.unlock();
}

static PyObject*
trans_prompt(Transaction* self, PyObject* args) {
    const char* path;
    PyObject* pyData;
    if (!PyArg_ParseTuple(args, "sO", &path, &pyData)) {
        return NULL;
    }
    // NEEDSWORK!!!  Python doesn't support blocks.  Should we use lambdas?
    // If so, check that a lambda was also used.
    bp::Object* bpData = pythonToBPObject(pyData);
    const BPElement* e = (bpData == NULL) ? NULL : bpData->elemPtr();
    s_lock.lock();
    unsigned int x = g_bpCoreFunctions->prompt(self->tid, path, e, dummyCB, NULL);
    // Grab the passed-in block and increment a reference to it.
    // NEEDSWORK!!!  Python doesn't support blocks.  Should we use lambdas?
    // Wait, python does support blocks.  but they are kinda second class.  hrm wtf
    // If so, use that lambda.
    PyObject* val = NULL;
    /*PyObject* val = rb_block_proc();*/
    s_outstandingPrompts[x] = val;
    // NEEDSWORK!!!  Python doesn't support blocks.  Should we use lambdas?
    // If so, uhh, not sure..  do we just do an INCREF? then a DECREF in the dummyCB?
    /*rb_gc_register_address(&s_outstandingPrompts[x]);*/
    s_lock.unlock();
    // now call the block!
    if (bpData != NULL) {
        delete bpData;
    }
    Py_RETURN_NONE;
}

static PyObject*
trans_error(Transaction* self, PyObject* args) {
    const char* error = "unknownError";
    const char* verboseError = NULL;
    if (!PyArg_ParseTuple(args, "|ss", &error, &verboseError)) {
        return NULL;
    }
    g_bpCoreFunctions->postError(self->tid, error, verboseError);
    Py_RETURN_NONE;
}

static PyObject*
trans_tid(Transaction* self) {
    return PyLong_FromLong(self->tid);
}

static PyMethodDef Transaction_methods[] = {
    {"initialize", (PyCFunction)trans_init, METH_VARARGS, "Initialize"},
    {"complete", (PyCFunction)trans_complete, METH_VARARGS, "Complete"},
    {"prompt", (PyCFunction)trans_prompt, METH_VARARGS, "Prompt"},
    {"error", (PyCFunction)trans_error, METH_VARARGS, "Error"},
    {"tid", (PyCFunction)trans_tid, METH_NOARGS, "Transaction ID"},
    {NULL} /* Sentinel */
};

static PyTypeObject TransactionType = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /*ob_size*/
    "browserplus.Transaction",                /*tp_name*/
    sizeof(Transaction),                      /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)Transaction_dealloc,          /*tp_dealloc*/
    0,                                        /*tp_print*/
    0,                                        /*tp_getattr*/
    0,                                        /*tp_setattr*/
    0,                                        /*tp_compare*/
    0,                                        /*tp_repr*/
    0,                                        /*tp_as_number*/
    0,                                        /*tp_as_sequence*/
    0,                                        /*tp_as_mapping*/
    0,                                        /*tp_hash*/
    0,                                        /*tp_call*/
    0,                                        /*tp_str*/
    0,                                        /*tp_getattro*/
    0,                                        /*tp_setattro*/
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Transaction objects",                    /*tp_doc*/
    0,                                        /*tp_traverse*/
    0,                                        /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    Transaction_methods,                      /*tp_methods*/
    Transaction_members,                      /*tp_members*/
    0,                                        /*tp_getset*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)Transaction_init,               /*tp_init*/
    0,                                        /*tp_alloc*/
    Transaction_new,                          /*tp_new*/
};
/*****************************/
/***  END TRANSACTION DEF  ***/
/*****************************/

/*****************************/
/***  BEGIN CALLBACK DEF   ***/
/*****************************/
typedef struct {
    PyObject_HEAD
    unsigned int tid;
    unsigned int cid;
} Callback;

static void
Callback_dealloc(Callback* self) {
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject*
Callback_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Callback* self = (Callback*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->tid = 0;
        self->cid = 0;
    }
    return (PyObject*)self;
}

static int
Callback_init(Callback* self, PyObject* args, PyObject* kwds) {
    static char* kwlist[] = {"tid", "cid", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ii", kwlist, &self->tid, &self->cid)) {
        return -1;
    }
    return 0;
}

static PyMemberDef Callback_members[] = {
    {"tid", T_LONG, offsetof(Callback, tid), 0, "Transaction ID"},
    {"cid", T_LONG, offsetof(Callback, cid), 0, "Callback ID"},
    {NULL} /* Sentinel */
};

static PyObject*
callb_init(Callback* self, PyObject* args) {
    unsigned int tid;
    unsigned int cid;
    if (!PyArg_ParseTuple(args, "II", &tid, &cid)) {
        return NULL;
    }
    self->tid = tid;
    self->cid = cid;
    Py_RETURN_NONE;
}

static PyObject*
callb_invoke(Callback* self, PyObject* args) {
    PyObject* pyData;
    if (!PyArg_ParseTuple(args, "O", &pyData)) {
        return NULL;
    }
    bp::Object* bpData = pythonToBPObject(pyData);
    const BPElement* e = (bpData == NULL) ? NULL : bpData->elemPtr();
    g_bpCoreFunctions->invoke(self->tid, self->cid, e);
    if (bpData != NULL) {
        delete bpData;
    }
    Py_RETURN_NONE;
}

static PyObject*
callb_tid(Callback* self) {
    return PyLong_FromLong(self->tid);
}

static PyObject*
callb_cid(Callback* self) {
    return PyLong_FromLong(self->cid);
}

static PyMethodDef Callback_methods[] = {
    {"initialize", (PyCFunction)callb_init, METH_VARARGS, "Initialize"},
    {"invoke", (PyCFunction)callb_invoke, METH_VARARGS, "Invoke"},
    {"tid", (PyCFunction)callb_tid, METH_NOARGS, "Transaction ID"},
    {"cid", (PyCFunction)callb_cid, METH_NOARGS, "Callback ID"},
    {NULL} /* Sentinel */
};

static PyTypeObject CallbackType = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /*ob_size*/
    "browserplus.Callback",                   /*tp_name*/
    sizeof(Callback),                         /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)Callback_dealloc,             /*tp_dealloc*/
    0,                                        /*tp_print*/
    0,                                        /*tp_getattr*/
    0,                                        /*tp_setattr*/
    0,                                        /*tp_compare*/
    0,                                        /*tp_repr*/
    0,                                        /*tp_as_number*/
    0,                                        /*tp_as_sequence*/
    0,                                        /*tp_as_mapping*/
    0,                                        /*tp_hash*/
    0,                                        /*tp_call*/
    0,                                        /*tp_str*/
    0,                                        /*tp_getattro*/
    0,                                        /*tp_setattro*/
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Callback objects",                       /*tp_doc*/
    0,                                        /*tp_traverse*/
    0,                                        /*tp_clear*/
    0,                                        /*tp_richcompare*/
    0,                                        /*tp_weaklistoffset*/
    0,                                        /*tp_iter*/
    0,                                        /*tp_iternext*/
    Callback_methods,                         /*tp_methods*/
    Callback_members,                         /*tp_members*/
    0,                                        /*tp_getset*/
    0,                                        /*tp_base*/
    0,                                        /*tp_dict*/
    0,                                        /*tp_descr_get*/
    0,                                        /*tp_descr_set*/
    0,                                        /*tp_dictoffset*/
    (initproc)Callback_init,                  /*tp_init*/
    0,                                        /*tp_alloc*/
    Callback_new,                             /*tp_new*/
};
/*****************************/
/***   END CALLBACK DEF    ***/
/*****************************/

static PyMethodDef module_methods[] = {
    {NULL} /* Sentinel */
};

void
bp_load_builtins() {
    static bool initd = false;
    assert(!initd);
    initd = true;
    if (PyType_Ready(&TransactionType) < 0) {
        return;
    }
    if (PyType_Ready(&CallbackType) < 0) {
        return;
    }
    PyObject* m = Py_InitModule3("browserplus", module_methods, "BrowserPlus Built-Ins.");
    if (m == NULL) {
        return;
    }
    Py_XINCREF(&TransactionType);
    Py_XINCREF(&CallbackType);
    PyModule_AddObject(m, "Transaction", (PyObject*)&TransactionType);
    PyModule_AddObject(m, "Callback", (PyObject*)&CallbackType);
    bp_py_cTransaction = (PyObject*)&TransactionType;;
    bp_py_cCallback = (PyObject*)&CallbackType;
}
