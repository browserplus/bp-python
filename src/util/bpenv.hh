/**
 * ***** BEGIN LICENSE BLOCK *****
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * The Original Code is BrowserPlus (tm).
 * 
 * The Initial Developer of the Original Code is Yahoo!.
 * Portions created by Yahoo! are Copyright (C) 2006-2010 Yahoo!.
 * All Rights Reserved.
 * 
 * Contributor(s): 
 * ***** END LICENSE BLOCK *****
 */

/*
 *  bpenv.h
 *
 *  Lightweight cross-platform abstractions of environment variables.
 *
 *  Created by Gary MacDonald on 6/2/10.
 */

#ifndef __BPENV_H__
#define __BPENV_H__

#include <string>

namespace bp { 
namespace env {
    std::string getEnvVar(const std::string& varName);
    void setEnvVar(const std::string& varName, const std::string& varValue);
}}

#endif
