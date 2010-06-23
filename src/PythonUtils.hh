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
 * PythonUtils.hh: Wrappers for including python.
 */

#ifndef __PYTHONUTILS_H__
#define __PYTHONUTILS_H__

#include "PythonHeaders.hh"
#include <string>
#include <map>

namespace python {
    /**
     * Convert forward-slash style path to native path.  HACKY.
     */
    std::string convertPathToNative(const std::string& s);
    /**
     * Get the last error encountered from the python evaluation environment.
     */
    std::string getLastError();
    /**
     * Simplified invocation of a function upon a reciever while catching
     * errors.
     */
    PyObject* invokeFunction(PyObject *r, const char* funcName, int* error, int nargs, ...);
    /**
     * Simplified invocation of a function upon a reciever while catching
     * errors.
     */
    PyObject* invokeMethod(PyObject* r, const char* funcName, int* error, int nargs, ...);
    // This little class is taken from http://metaeditor.sourceforge.net/embed/
    // The idea is simple, we need anonymous values returned from
    // python to have a non-zero reference count so that they are not
    // deleted.
    // We allocate an array on the stack, and add elements to this array,
    // because the array is marked, those items will not be deleted
    // as long as they're in the array.
    class GCArray {
    public:
        GCArray() {
        }
        ~GCArray() {
            // Dispose array and flush all elements.
            while (!objects.empty()) {
                std::map<PyObject*, PyObject*>::iterator i = objects.begin();
                Py_XDECREF(i->second);
                objects.erase(i);
            }
        }
        void Register(PyObject* object) {
            objects[object] = object;
            Py_XINCREF(object);
        }
        void Unregister(PyObject* object) {
            for (std::map<PyObject*, PyObject*>::iterator i = objects.begin(); i != objects.end(); i++) {
                if (i->first == object) {
                    Py_XDECREF(i->second);
                    objects.erase(i);
                    break;
                }
            }
        }
    private:
        std::map<PyObject*, PyObject*> objects;
    };
};

#endif // __PYTHONUTILS_H__
