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
 *
 */

#include "PythonUtils.hh"

#include <assert.h>

#include "PythonHeaders.hh"

std::string python::getLastError()
{
    PyObject* lasterr = rb_gv_get("$!");
    PyObject* errMessage = rb_obj_as_string(lasterr);
    return RSTRING_PTR(errMessage);
}

#define MAX_ARGS 32

typedef struct 
{
    PyObject* receiver;
    ID function;
    int nargs;
    PyObject* args[MAX_ARGS];
} FuncallArgs;

static PyObject* rb_funcall_proxy(PyObject* arg)
{
    FuncallArgs * fa = (FuncallArgs *) arg;
    return rb_funcall2(fa->receiver,
                       fa->function,
                       fa->nargs,
                       fa->args);
}
    
PyObject*
python::invokeFunction(PyObject* r, const char * funcName, int * error,
                     int nargs, ...)
{
    FuncallArgs fa;
    
    fa.receiver = r;
    fa.function = rb_intern(funcName);
    fa.nargs = nargs;

    *error = 0;

    va_list argh;
    assert(nargs < MAX_ARGS);
    
    va_start(argh, nargs);
    for (int i = 0; i < nargs; i++) fa.args[i] = va_arg(argh, PyObject*);
    va_end(argh);
    
    return rb_protect(rb_funcall_proxy, (PyObject*) &fa, error);
}
#endif // 0
