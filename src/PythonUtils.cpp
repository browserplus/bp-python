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
#include "PythonUtils.hh"

#ifdef WIN32
#define PATHDELIM '\\'
#else // WIN32
#define PATHDELIM '/'
#endif // WIN32

std::string python::convertPathToNative(const std::string& s) {
    std::string s2;
    for (std::string::const_iterator i = s.begin(); i != s.end(); i++) {
        if (*i == '/') {
            s2.push_back(PATHDELIM);
        }
        else {
            s2.push_back(*i);
        }
    }
    return s2;
}

std::string python::getLastError() {
    std::string s;
    PyObject* lasterr = PyErr_Occurred();
    if (lasterr == NULL) {
        return s;
    }
    PyObject* ptype;
    PyObject* pvalue;
    PyObject* ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    if (pvalue != NULL) {
        PyObject* errstr = PyObject_Str(pvalue);
        s = PyString_AsString(errstr);
        Py_XDECREF(errstr);
    }
    PyErr_Restore(ptype, pvalue, ptraceback);
    return s;
}

#define MAX_ARGS 32

PyObject*
python::invokeFunction(PyObject *r, const char* funcName, int* error, int nargs, ...)
{
    // NEEDSWORK!!!  We're assuming 'r' is not NULL and not Py_None.
    // Should we validate and throw an Py Exception otherwise?
    *error = 0;
    assert(nargs < MAX_ARGS);
    va_list argh;
    va_start(argh, nargs);
	PyObject* pyArgs[MAX_ARGS];
	for (int i = 0; i < nargs; i++) {
		pyArgs[i] = va_arg(argh, PyObject*);
	}
	PyObject* f = PyObject_GetAttrString(r, funcName);
    PyObject* result;
    switch (nargs) {
        case 0:
            result = PyObject_CallFunctionObjArgs(f, NULL);
            break;
        case 1:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], NULL);
            break;
        case 2:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], NULL);
            break;
        case 3:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], NULL);
            break;
        case 4:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], NULL);
            break;
        case 5:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], NULL);
            break;
        case 6:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], NULL);
            break;
        case 7:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], NULL);
            break;
        case 8:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], NULL);
            break;
        case 9:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], NULL);
            break;
        case 10:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], NULL);
            break;
        case 11:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], NULL);
            break;
        case 12:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], NULL);
            break;
        case 13:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], NULL);
            break;
        case 14:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], NULL);
            break;
        case 15:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], NULL);
            break;
        case 16:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], NULL);
            break;
        case 17:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], NULL);
            break;
        case 18:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], NULL);
            break;
        case 19:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], NULL);
            break;
        case 20:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], NULL);
            break;
        case 21:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], NULL);
            break;
        case 22:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], NULL);
            break;
        case 23:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], NULL);
            break;
        case 24:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], NULL);
            break;
        case 25:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], NULL);
            break;
        case 26:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], NULL);
            break;
        case 27:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], NULL);
            break;
        case 28:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], NULL);
            break;
        case 29:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], pyArgs[28], NULL);
            break;
        case 30:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], pyArgs[28], pyArgs[29], NULL);
            break;
        case 31:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], pyArgs[28], pyArgs[29], pyArgs[30], NULL);
            break;
        case 32:
            result = PyObject_CallFunctionObjArgs(f, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], pyArgs[28], pyArgs[29], pyArgs[30], pyArgs[31], NULL);
            break;
        default:
            result = NULL;
            // NEEDSWORK!!!  We should probably do a python exception here?
            break;
    }
    va_end(argh);
	if (result == NULL)
	{
		std::string s = python::getLastError();
	}
    Py_XDECREF(f);
    return result;
}

PyObject*
python::invokeMethod(PyObject* r, const char* funcName, int* error, int nargs, ...)
{
    // NEEDSWORK!!!  We're assuming 'r' is not NULL and not Py_None.
    // Should we validate and throw an Py Exception otherwise?
    *error = 0;
    assert(nargs < MAX_ARGS);
    va_list argh;
    va_start(argh, nargs);
	PyObject* pyArgs[MAX_ARGS];
	for (int i = 0; i < nargs; i++) {
		pyArgs[i] = va_arg(argh, PyObject*);
	}
    PyObject* funcNameString = PyString_FromString(funcName);
    PyObject* result;
    switch (nargs) {
        case 0:
            result = PyObject_CallMethodObjArgs(r, funcNameString, NULL);
            break;
        case 1:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], NULL);
            break;
        case 2:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], NULL);
            break;
        case 3:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], NULL);
            break;
        case 4:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], NULL);
            break;
        case 5:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], NULL);
            break;
        case 6:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], NULL);
            break;
        case 7:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], NULL);
            break;
        case 8:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], NULL);
            break;
        case 9:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], NULL);
            break;
        case 10:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], NULL);
            break;
        case 11:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], NULL);
            break;
        case 12:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], NULL);
            break;
        case 13:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], NULL);
            break;
        case 14:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], NULL);
            break;
        case 15:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], NULL);
            break;
        case 16:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], NULL);
            break;
        case 17:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], NULL);
            break;
        case 18:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], NULL);
            break;
        case 19:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], NULL);
            break;
        case 20:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], NULL);
            break;
        case 21:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], NULL);
            break;
        case 22:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], NULL);
            break;
        case 23:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], NULL);
            break;
        case 24:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], NULL);
            break;
        case 25:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], NULL);
            break;
        case 26:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], NULL);
            break;
        case 27:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], NULL);
            break;
        case 28:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], NULL);
            break;
        case 29:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], pyArgs[28], NULL);
            break;
        case 30:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], pyArgs[28], pyArgs[29], NULL);
            break;
        case 31:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], pyArgs[28], pyArgs[29], pyArgs[30], NULL);
            break;
        case 32:
            result = PyObject_CallMethodObjArgs(r, funcNameString, pyArgs[0], pyArgs[1], pyArgs[2], pyArgs[3], pyArgs[4], pyArgs[5], pyArgs[6], pyArgs[7], pyArgs[8], pyArgs[9], pyArgs[10], pyArgs[11], pyArgs[12], pyArgs[13], pyArgs[14], pyArgs[15], pyArgs[16], pyArgs[17], pyArgs[18], pyArgs[19], pyArgs[20], pyArgs[21], pyArgs[22], pyArgs[23], pyArgs[24], pyArgs[25], pyArgs[26], pyArgs[27], pyArgs[28], pyArgs[29], pyArgs[30], pyArgs[31], NULL);
            break;
        default:
            result = NULL;
            // NEEDSWORK!!!  We should probably do a python exception here?
            break;
    }
    va_end(argh);
	if (result == NULL)
	{
		std::string s = python::getLastError();
	}
    Py_XDECREF(funcNameString);
    return result;
}
