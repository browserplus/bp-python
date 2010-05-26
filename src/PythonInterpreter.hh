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
 * PythonInterpreter.hh: Wrappers for including python.
 */

#ifndef __PYTHONINTERPRETER_H__
#define __PYTHONINTERPRETER_H__

#include "ServiceAPI/bptypes.h"
#include "bpservicedescription.hh"
#include <string>

namespace python {
    // Intialize the python interpreter, given the path to this service.
    // This will call all required initialization routines and
    // will correctly populate load paths.
    void initialize(const std::string& pathToPythonServiceDataDir);
    // Shutdown the python interpreter, freeing all possible resources.
    void shutdown();
    // Given a path to a entry point python file, load the file and
    // extract a description.  On error, NULL is returned and a human
    // readable error is returned in the oError output param.
    bp::service::Description* loadPythonService(const std::string& pathToPythonFile, std::string& oError);
    void* allocateInstance(const bp::Map* context);
    void invoke(void* instance, const char* funcName, unsigned int tid, bp::Map* arguments);
    void destroyInstance(void* instance);
}

#endif // PYTHONINTERPRETER_H__
