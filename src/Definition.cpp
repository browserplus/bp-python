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
 * Code to extract a service definition from a python service.
 */

#include "PythonHeaders.hh" // must be included before *any* other headers
#include "Definition.hh"
#include "PythonUtils.hh"
#include <sstream>

const char* python::BP_GLOBAL_DEF_SYM = "BrowserPlusEntryPointClass";
#define BP_EXTERNAL_REP_METHOD "to_service_description"

bool
extractString(PyObject* hash, const char* key, std::string& where) {
    bool result = false;
    where.clear();
    PyObject* rkey = PyString_FromString(key);
    PyObject* val = PyDict_GetItem(hash, rkey);
    Py_DECREF(rkey);
    if (PyString_Check(val)) {
        where.append(PyString_AsString(val));
        result = true;
    }
    return result;
}

bool
extractNumber(PyObject* hash, const char* key, unsigned int* where) {
    bool result = false;
    *where = 0;
    PyObject* rkey = PyString_FromString(key);
    PyObject* val = PyDict_GetItem(hash, rkey);
    Py_DECREF(rkey);
    if (PyInt_Check(val)) {
        *where = PyInt_AS_LONG(val);
        result = true;
    }
    else if (PyLong_Check(val)) {
        *where = PyLong_AsLong(val);
        result = true;
    }
    return result;
}

bool
extractBool(PyObject* hash, const char* key, bool* where) {
    bool result = false;
    *where = false;
    PyObject* rkey = PyString_FromString(key);
    PyObject* val = PyDict_GetItem(hash, rkey);
    Py_DECREF(rkey);
    if (PyBool_Check(val)) {
        if (val == Py_True) {
            *where = true;
            result = true;
        }
        else if (val == Py_False) {
            *where = false;
            result = true;
        }
    }
    return result;
}

bool
processArgument(PyObject* hash, bp::service::Function& f, std::string& verboseError) {
    bp::service::Argument a;
    std::string s;
    if (!extractString(hash, "name", s)) {
        verboseError.append("'name' missing from argument definition\n");
        return false;
    }
    a.setName(s.c_str());
    if (!extractString(hash, "documentation", s)) {
        verboseError.append("'documentation' missing from ");
        verboseError.append(a.name());
        verboseError.append(" argument definition\n");
        return false;
    }
    a.setDocString(s.c_str());
    bool required = false;
    if (!extractBool(hash, "required", &required)) {
        verboseError.append("'required' specification missing from ");
        verboseError.append(a.name());
        verboseError.append(" argument definition");
        return false;
    }
    a.setRequired(required);
    // Now for the type.
    if (!extractString(hash, "type", s)) {
        verboseError.append("'type' missing from ");
        verboseError.append(a.name());
        verboseError.append(" argument definition");
        return false;
    }
    // Map string to type.
    bp::service::Argument::Type t;
    t = bp::service::Argument::stringAsType(s.c_str());
    if (t == bp::service::Argument::None) {
        verboseError.append("invalid argument type: " + s);
        return false;
    }
    a.setType(t);
    // NEEDSWORK!!!  Need addArgument.
    std::list<bp::service::Argument> arguments = f.arguments();
    arguments.push_back(a);
    f.setArguments(arguments);
    return true;
}

static bool
processFunction(PyObject* hash, bp::service::Description* desc, std::string& verboseError) {
    bp::service::Function f;
    std::string s;
    if (!extractString(hash, "name", s)) {
        verboseError.append("'name' missing from function definition");
        return false;
    }
    f.setName(s.c_str());
    if (!extractString(hash, "documentation", s)) {
        verboseError.append("'documentation' missing from ");
        verboseError.append(f.name());
        verboseError.append(" function definition");
        return false;
    }
    f.setDocString(s.c_str());
    // Now process the arguments.
    {
        PyObject* rkey = PyString_FromString("arguments");
        PyObject* arr = PyDict_GetItem(hash, rkey);
        Py_DECREF(rkey);
        if (!PyList_Check(arr)) {
            verboseError.append("'arguments' array missing from definition of");
            verboseError.append(f.name());
            verboseError.append(" function");
            return false;
        }
        for (long int i = 0; true; i++) {
            PyObject* rfHash = PyList_GetItem(arr, i);
            if (rfHash == Py_None) {
                break;
            }
            if (!PyDict_Check(rfHash)) {
                verboseError.append("non-hash member found in 'arguments' array for function ");
                verboseError.append(f.name());
                return false;
            }
            // Now process the function hash.
            if (!processArgument(rfHash, f, verboseError)) {
                return false;
            }
        }
    }
    // NEEDSWORK!!!  Need "addFunction."
    std::list<bp::service::Function> l(desc->functions());
    l.push_back(f);
    desc->setFunctions(l);
    return true;
}

static std::string
formatError(const char* e) {
    std::stringstream ss;
    ss << e << ": " << python::getLastError();
    return ss.str();
}

bp::service::Description*
python::extractDefinition(std::string& verboseError)
{
    // Global variable "BrowserPlusEntryPointClass" will reference a
    // class with a .to_service_description method.  This method returns
    // a stable python data structure that we can traverse to build up a
    // c++/c representation of the services interface.
    PyObject* defSym = 0;
    {
        PyObject *m = PyImport_AddModule("__main__");
        PyObject *gv = PyObject_GetAttrString(m, BP_GLOBAL_DEF_SYM);
        // NEEDSWORK!!!  Is this correct?
        if (!PyClass_Check(gv)) {
            verboseError.append("python service lacks entry point class, cannot find ");
            verboseError.append(BP_GLOBAL_DEF_SYM);
            verboseError.append(", is bp_doc properly called?");
            Py_DECREF(gv);
            return NULL;
        }
        int error = 0;
        defSym = python::invokeFunction(gv, BP_EXTERNAL_REP_METHOD, &error, 0);
        if (error) {
            verboseError = formatError("couldn't attain service description");
            Py_XDECREF(defSym);
            Py_DECREF(gv);
            return NULL;
        }
        if (!PyDict_Check(defSym)) {
            verboseError.append(BP_EXTERNAL_REP_METHOD " returns invalid type");
            Py_DECREF(defSym);
            Py_DECREF(gv);
            return NULL;
        }
        Py_DECREF(gv);
    }
    // Now we have a HASH ready to traverse!
    bp::service::Description* desc = new bp::service::Description;
    // First grab the name of the service.
    std::string s;
    if (!extractString(defSym, "name", s)) {
        verboseError.append("'name' missing from service description");
        delete desc;
        Py_DECREF(defSym);
        return NULL;
    }
    desc->setName(s.c_str());
    if (!extractString(defSym, "documentation", s)) {
        verboseError.append("'documentation' missing from service description");
        delete desc;
        Py_DECREF(defSym);
        return NULL;
    }
    desc->setDocString(s.c_str());
    if (!extractString(defSym, "version", s)) {
        verboseError.append("'version' missing from service description");
        delete desc;
        Py_DECREF(defSym);
        return NULL;
    }
    // Now parse the version.
    {
        bp::ServiceVersion v;
        if (!v.parse(s)) {
            verboseError.append("malformed 'version' string");
            delete desc;
            Py_DECREF(defSym);
            return NULL;
        }
        desc->setMajorVersion(v.majorVer());
        desc->setMinorVersion(v.minorVer());
        desc->setMicroVersion(v.microVer());
    }
    // Now process the functions.
    {
        PyObject* rkey = PyString_FromString("functions");
        PyObject* arr = PyDict_GetItem(defSym, rkey);
        Py_DECREF(rkey);
        if (!PyList_Check(arr)) {
            verboseError.append("'functions' array missing from service description");
            delete desc;
            Py_DECREF(defSym);
            return NULL;
        }
        for (long int i = 0; true; i++) {
            PyObject* rfHash = PyList_GetItem(arr, i);
            if (rfHash == Py_None) {
                break;
            }
            if (!PyDict_Check(rfHash)) {
                verboseError.append("non-hash member found in 'functions' array\n");
                delete desc;
                Py_DECREF(defSym);
                return NULL;
            }
            // Now process the function hash.
            if (!processFunction(rfHash, desc, verboseError)) {
                delete desc;
                Py_DECREF(defSym);
                return NULL;
            }
        }
    }
    Py_DECREF(defSym);
    return desc;
}
