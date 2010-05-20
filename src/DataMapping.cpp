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

/**
 * DataMapping.cpp - tools to map data back and forth from python's PyObject* to
 *                 bp::Object 
 */

#include "DataMapping.hh"
#include "PythonUtils.hh"
#include "BuiltIns.hh"

#include "bpurlutil.hh"

bp::Object *
pythonToBPObject(PyObject* v)
{
    bp::Object * obj = NULL;

    // Python APIs don't have NULL guards, so we must do it.
    if (v == NULL) {
        obj = new bp::Null();
    }
    else if (PyBool_Check(v)) {
        if (v == Py_True) {
            obj = new bp::Bool(true);
        }
        else if (v == Py_False) {
            obj = new bp::Bool(false);
        }
    }
    else if (PyInt_Check(v)) {
        obj = new bp::Integer(PyInt_AS_LONG(v));
    }
    else if (PyLong_Check(v)) {
        obj = new bp::Integer(PyInt_AsLong(v));
    }
    else if (PyFloat_Check(v)) {
        obj = new bp::Double(PyFloat_AS_DOUBLE(v));
    }
    else if (PyString_Check(v)) {
        obj = new bp::String(PyString_AS_STRING(v));
    }
    else if (PyDict_Check(v)) {
        bp::Map * m = new bp::Map;
        PyObject* key;
        PyObject* value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(v, &pos, &key, &value)) {
            m->add(PyString_AS_STRING(key), pythonToBPObject(value));
        }
        obj = m;
    }
    else if (PyList_Check(v)) {
        Py_ssize_t i;
        bp::List * l = new bp::List;
        for (i=0; i < PyList_GET_SIZE(v); i++) {
            l->append(pythonToBPObject(PyList_GET_ITEM(v, i)));
        }
        obj = l;
    }
    else {
        obj = new bp::Null();
    }
///////
#if 0
    switch (TYPE(v)) {
        case T_OBJECT: {
            // map Pathname objects into BPTPath types
            PyObject* id = rb_intern("Pathname");
            PyObject* klass = 0;
            if (rb_const_defined(rb_cObject, id) &&
                (klass = rb_const_get(rb_cObject, id)) &&
                TYPE(klass) == T_CLASS)
            {
                PyObject* r = rb_obj_is_kind_of(v, klass);
                if (RTEST(r)) {
                    // convert to abs path
                    int error = 0;
                    PyObject* absPath =
                        python::invokeFunction(v, "realpath", &error, 0);
                    PyObject* pathString =
                        python::invokeFunction(absPath, "to_s", &error, 0);
                    if (!error && TYPE(pathString) == T_STRING) {
                        std::string uri =
                            bp::urlutil::urlFromPath(RSTRING_PTR(pathString));
                        obj = new bp::Path(uri);
                    }
                    break;
                }
            }
        }
    }
#endif // 0
    return obj;
}

unsigned long int /*PyObject**/
bpObjectToPython(const bp::Object * obj,
               unsigned int tid)
{
    if (obj == NULL) {
        //(unsigned long int)Py_RETURN_NONE;
        return (unsigned long int)Py_None;
    }

    // To avoid multiple allocations, temporarily use NULL instead of Py_None
    PyObject* v = NULL;

    switch (obj->type()) {
        case BPTNull:
            v = NULL;
            break;
        case BPTBoolean:
        {
            if (((bp::Bool *) obj)->value()) v = Py_True;
            else v = Py_False;
            break;
        }
        case BPTInteger:
            v = PyInt_FromLong(((bp::Integer *) obj)->value());
            break;
        case BPTDouble:
            v = PyFloat_FromDouble(((bp::Double *) obj)->value());
            break;
        case BPTString:
            v = PyString_FromString(((bp::String *) obj)->value());
            break;
        case BPTMap: 
        {
            bp::Map * m = (bp::Map *) obj;
            v = PyDict_New();
            bp::Map::Iterator i(*m);
            const char * key;
            while (NULL != (key = i.nextKey())) {
                PyDict_SetItem(v,PyString_FromString(key), 
                             (PyObject*)bpObjectToPython(m->value(key), tid));
            }
            
            break;
        }
        
        case BPTList: 
        {
            bp::List * l = (bp::List *) obj;

            v = PyList_New(l->size());
            
            unsigned int i;
            for (i=0; i < l->size(); i++) {
                PyList_Append(v, (PyObject*)bpObjectToPython(l->value(i), tid));
            }
            
            break;
        }
#if 0
        case BPTPath: {
            PyObject* id = rb_intern("Pathname");
            PyObject* klass = 0;
            if (rb_const_defined(rb_cObject, id) &&
                (klass = rb_const_get(rb_cObject, id)) &&
                TYPE(klass) == T_CLASS)
            {
                std::string url = ((bp::Path *) obj)->value();
                std::string path = bp::urlutil::pathFromURL(url);
                PyObject* p = rb_str_new2(path.c_str());
                v = rb_class_new_instance(1, &p, klass);
            }
            break;
        }
#endif // 0
#if 0
        case BPTCallBack: {
            PyObject* args[2];
            args[0] = rb_uint2inum(tid);
            args[1] = rb_ull2inum(((bp::Integer *) obj)->value());
            v = rb_class_new_instance(2, args, bp_rb_cCallback);
            break;
        }           
#endif // 0
        case BPTAny: 
            // invalid
            break;
    }

    // To avoid multiple allocations
    if (v == NULL) {
        //(unsigned long int)Py_RETURN_NONE;
        return (unsigned long int)Py_None;
    }
    return (unsigned long int)v;
}
