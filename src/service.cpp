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
#include "PythonInterpreter.hh"
#include "PythonUtils.hh"
#include "ServiceGlobals.hh"
// Header files from the bp-service-tools project, which makes it
// easier to deal in types that one may transmit across the service
// boundary.
#include "bptypeutil.hh"
#include <map>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

// a description of this service.
BPCoreletDefinition s_pythonInterpreterDef = {
    "PythonInterpreter",
    1, 0, 0,
    "Allows other services to be written in Python.",
    0,
    NULL
};

const BPCFunctionTable* g_bpCoreFunctions;

#ifdef WIN32
#include <windows.h>
#endif // WIN32

const BPCoreletDefinition*
BPPInitialize(const BPCFunctionTable* bpCoreFunctions, const BPElement* parameterMap) {
#ifdef WIN32
    DebugBreak();
#endif // WIN32
    // The name of the python script and path can be extracted from the parameter map.
    bp::Object* obj = bp::Object::build(parameterMap);
    // First get the path.
    if (!obj->has("CoreletDirectory", BPTString)) {
        delete obj;
        return NULL;
    }
    std::string path(python::convertPathToNative(((bp::String*)obj->get("CoreletDirectory"))->value()));
    delete obj;
    g_bpCoreFunctions = bpCoreFunctions;
    // This will go in the BrowserPlusCore log file at info level.  Nice.
    g_bpCoreFunctions->log(BP_INFO, "initializing python interpreter with service path: %s", path.c_str());
    // Now let's initialize the python Interpreter.
    (void)python::initialize(path);
    return &s_pythonInterpreterDef;
}

void
BPPShutdown(void) {
    python::shutdown();
}

int
BPPAllocate(void** instance, unsigned int, const BPElement* contextMap) {
    bp::Object* obj = bp::Object::build(contextMap);
    *instance = python::allocateInstance(dynamic_cast<bp::Map*>(obj));
    if (obj) {
        delete obj;
    }
    // NEEDSWORK!!! Failure case?
    return 0;
}

void
BPPDestroy(void* instance) {
    python::destroyInstance(instance);
}

void
BPPInvoke(void* instance, const char* funcName, unsigned int tid, const BPElement* elem) {
    bp::Object* obj = bp::Object::build(elem);
    python::invoke(instance, funcName, tid, dynamic_cast<bp::Map*>(obj));
    if (obj) {
        delete obj;
    }
}

// File scoped memory representation of the services interface.
static bp::service::Description* s_desc = NULL;

#ifdef WIN32
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

const BPCoreletDefinition*
BPPAttach(unsigned int attachID, const BPElement* paramMap) {
    const BPCoreletDefinition* def = NULL;
    if (def != NULL) {
        return def;
    }
    // The name of the python script and path can be extracted from the parameter map.
    bp::Object* obj = bp::Object::build(paramMap);
    // First get the path.
    if (!obj->has("CoreletDirectory", BPTString)) {
        delete obj;
        return NULL;
    }
    std::string path;
    path.append(python::convertPathToNative(((bp::Path*)obj->get("CoreletDirectory"))->value()));
    // Now get the script name.
    if (!obj->has("Parameters", BPTMap)) {
        delete obj;
        return NULL;
    }
    bp::Map* params = (bp::Map*)obj->get("Parameters");
    if (!params->has("ScriptFile", BPTString)) {
        delete obj;
        return NULL;
    }
    path.append(PATHSEP);
    path.append(((bp::Path*)params->get("ScriptFile"))->value());
    std::string error;
    s_desc = python::loadPythonService(path, error);
    if (s_desc == NULL) {
        g_bpCoreFunctions->log(BP_ERROR, "error loading python service: %s", error.c_str());
        return NULL;
    }
    return (def = s_desc->toBPCoreletDefinition());
}

void
BPPDetach(unsigned int attachID) {
    if (s_desc) {
        delete s_desc;
    }
    s_desc = NULL;
}

/**
 * And finally, declare the entry point to the service.
 */
BPPFunctionTable funcTable = {
    BPP_CORELET_API_VERSION,
    BPPInitialize,
    BPPShutdown,
    BPPAllocate,
    BPPDestroy,
    BPPInvoke,
    BPPAttach,
    BPPDetach
};

const BPPFunctionTable*
BPPGetEntryPoints(void) {
    return &funcTable;
}
