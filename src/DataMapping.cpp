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
#include "DataMapping.hh"
#include "PythonUtils.hh"
#include "BuiltIns.hh"
#include "bpurlutil.hh"

bp::Object*
pythonToBPObject(void* v) {
    bp::Object* obj = NULL;
    PyObject* pv = (PyObject*)v;
    if (pv == NULL) {
        obj = NULL;
    }
    else if (pv == Py_None) {
        obj = new bp::Null();
    }
    else if (PyBool_Check(pv)) {
        if (pv == Py_True) {
            obj = new bp::Bool(true);
        }
        else if (pv == Py_False) {
            obj = new bp::Bool(false);
        }
    }
    else if (PyInt_Check(pv)) {
        obj = new bp::Integer(PyInt_AS_LONG(pv));
    }
    else if (PyLong_Check(pv)) {
        obj = new bp::Integer(PyInt_AsLong(pv));
    }
    else if (PyFloat_Check(pv)) {
        obj = new bp::Double(PyFloat_AS_DOUBLE(pv));
    }
    else if (PyString_Check(pv)) {
        obj = new bp::String(PyString_AS_STRING(pv));
        // NEEDSWORK!!!  There is no pathname class in python.
        // And no way to tell if a string is a path unless the file/dir/link exists.
        // What should we do here?  We can't ever convert back to a BPPath?
    }
    else if (PyDict_Check(pv)) {
        bp::Map* m = new bp::Map;
        Py_ssize_t pos = 0;
        PyObject* key;
        PyObject* value;
        while (PyDict_Next(pv, &pos, &key, &value)) {
            m->add(PyString_AS_STRING(key), pythonToBPObject(value));
        }
        obj = m;
    }
    else if (PyList_Check(pv)) {
        bp::List* l = new bp::List;
        Py_ssize_t i;
        for (i = 0; i < PyList_GET_SIZE(pv); i++) {
            l->append(pythonToBPObject(PyList_GET_ITEM(pv, i)));
        }
        obj = l;
    }
    else {
        // Fallback.
        obj = new bp::Null();
    }
    return obj;
}

void* /*PyObject**/
bpObjectToPython(const bp::Object* obj, unsigned int tid) {
    if (obj == NULL) {
        Py_XINCREF(Py_None);
        return (void*)Py_None;
    }
    // To avoid multiple allocations, temporarily use NULL instead of Py_None.
    PyObject* v = NULL;
    switch (obj->type()) {
        case BPTNull:
            Py_XINCREF(Py_None);
            v = Py_None;
            break;
        case BPTBoolean:
            if (((bp::Bool*)obj)->value()) {
                Py_XINCREF(Py_True);
                v = Py_True;
            }
            else {
                Py_XINCREF(Py_False);
                v = Py_False;
            }
            break;
        case BPTInteger:
            v = PyInt_FromLong(((bp::Integer*)obj)->value());
            break;
        case BPTDouble:
            v = PyFloat_FromDouble(((bp::Double*)obj)->value());
            break;
        case BPTString:
            v = PyString_FromString(((bp::String*)obj)->value());
            break;
        case BPTMap: {
            bp::Map* m = (bp::Map*)obj;
            v = PyDict_New();
            bp::Map::Iterator i(*m);
            const char* key;
            while (NULL != (key = i.nextKey())) {
                PyDict_SetItem(v, PyString_FromString(key), (PyObject*)bpObjectToPython(m->value(key), tid));
            }
            break;
        }
        case BPTList: {
            bp::List* l = (bp::List*)obj;
            v = PyList_New(l->size());
            unsigned int i;
            for (i = 0; i < l->size(); i++) {
                PyList_Append(v, (PyObject*)bpObjectToPython(l->value(i), tid));
            }
            break;
        }
        case BPTPath: {
            // NEEDSWORK!!!  There is no path class in Python.
            // All we can do is represent this as a string.
            // This is a lossy translation.  Are there alternatives?
            std::string url = ((bp::Path*)obj)->value();
            std::string path = bp::urlutil::pathFromURL(url);
            v = PyString_FromString(path.c_str());
            break;
        }
        case BPTCallBack: {
            PyObject* args = Py_BuildValue("ll", tid, ((bp::Integer*)obj)->value());
            PyObject* kwds = Py_BuildValue("");
            v = PyType_GenericNew((PyTypeObject*)bp_py_cCallback, args, kwds);
            Py_XDECREF(kwds);
            Py_XDECREF(args);
            break;
        }
        case BPTAny:
            // Invalid.
            break;
    }
    // To avoid multiple allocations.
    if (v == NULL) {
        Py_XINCREF(Py_None);
        v = Py_None;
    }
    return (void*)v;
}
