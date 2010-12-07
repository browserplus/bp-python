#####
# A CMake build file for the PythonInterpreter service.
# 
# 
# Copyright 2009, Yahoo!
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
# 
#  3. Neither the name of Yahoo! nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# 
# Original Author: Gary MacDonald
#####

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1)

SET (SERVICE_NAME PythonInterpreter)
PROJECT(${SERVICE_NAME}Service)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../external/bp-build-templates/cmake/BuildConfigs.cmake)

# build up the "platform identifier"
SET(arch "i386")
IF (APPLE)
  EXECUTE_PROCESS(COMMAND uname -p
                  OUTPUT_VARIABLE arch
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  EXECUTE_PROCESS(COMMAND uname -r
                  OUTPUT_VARIABLE ver
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  EXECUTE_PROCESS(COMMAND uname -s
                  OUTPUT_VARIABLE sysUpper
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(TOLOWER ${sysUpper} sys)
  SET(platIdent "${arch}-${sys}${ver}")
  MESSAGE("platform: ${platIdent}")
ELSEIF (UNIX)
  EXECUTE_PROCESS(COMMAND uname -m
                  OUTPUT_VARIABLE arch
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  EXECUTE_PROCESS(COMMAND uname -s
                  OUTPUT_VARIABLE sysUpper
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  STRING(TOLOWER ${sysUpper} sys) 
  SET(platIdent "${arch}-${sys}")
  MESSAGE("platform: ${platIdent}")
ELSE ()
  # hard coded for now!
  SET(platIdent "i386-mswin32_90")
ENDIF ()

SET(extDir "${CMAKE_CURRENT_SOURCE_DIR}/../external")
SET(extBinDir "${extDir}/dist/bin")
#IF ("${CMAKE_BUILD_TYPE}" STREQUAL "CodeCoverage")
#  SET(extLibDir "${extDir}/dist/lib/Debug")
#ELSE ()
#  SET(extLibDir "${extDir}/dist/lib/${CMAKE_BUILD_TYPE}")
#ENDIF ()
SET(extLibDir "${extDir}/dist/lib")
SET(extIncDir "${extDir}/dist/include/python26")
SET(CPPTOOLS_DIR "${extDir}/bp-service-tools/CppTools/src")

INCLUDE_DIRECTORIES(
  "${extIncDir}/${platIdent}"
  "${extIncDir}"
  "${extDir}/bp-service-tools/CppTools/external/bpsdk/include"
  "${CPPTOOLS_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

SET(pythonLibPath ${extLibDir})
LINK_DIRECTORIES(${pythonLibPath})

# os specific srcs
IF (WIN32)
   FILE(GLOB OS_SRCS "util/*_Windows.cpp")
ELSE()
   FILE(GLOB OS_SRCS "util/*_Darwin.cpp" "util/*_UNIX.cpp")
ENDIF ()

SET(EXT_SRCS
  "${CPPTOOLS_DIR}/bptypeutil.cpp"
  "${CPPTOOLS_DIR}/bpservicedescription.cpp"
  "${CPPTOOLS_DIR}/bpserviceversion.cpp"
  "${CPPTOOLS_DIR}/bpurlutil.cpp"
)

#FILE(GLOB_RECURSE HDRS "*.hh")

# find the path to the python library we'll link
MESSAGE("looking in ${pythonLibPath}")
FIND_LIBRARY(pythonDylib NAMES python2.6
             PATHS ${pythonLibPath}
             NO_DEFAULT_PATH
             NO_SYSTEM_ENVIRONMENT_PATH)
GET_FILENAME_COMPONENT(pythonDylibName ${pythonDylib} NAME)
## find the path to the python library we'll link
#MESSAGE("looking in ${pythonLibPath}")
#IF(WIN32)
#  FIND_LIBRARY(pythonDylib_debug NAMES python2.6_d python26_d
#               PATHS ${extLibDir_debug}
#               NO_DEFAULT_PATH
#               NO_SYSTEM_ENVIRONMENT_PATH)
#  GET_FILENAME_COMPONENT(pythonDylibName_debug ${pythonDylib_debug} NAME)
#  FIND_LIBRARY(pythonDylib_release NAMES python2.6 python26
#               PATHS ${extLibDir_release}
#               NO_DEFAULT_PATH
#               NO_SYSTEM_ENVIRONMENT_PATH)
#  GET_FILENAME_COMPONENT(pythonDylibName_release ${pythonDylib_release} NAME)
#
#  TARGET_LINK_LIBRARIES(${SERVICE_NAME}
#                        debug ${pythonDylibName_debug}
#                        optimized ${pythonDylibName_release})
#ELSE(WIN32)
#  FIND_LIBRARY(pythonDylib NAMES python2.6 python26
#               PATHS ${pythonLibPath}
#               NO_DEFAULT_PATH
#               NO_SYSTEM_ENVIRONMENT_PATH)
#  GET_FILENAME_COMPONENT(pythonDylibName ${pythonDylib} NAME)
#
#  TARGET_LINK_LIBRARIES(${SERVICE_NAME} ${pythonDylibName})
#ENDIF(WIN32)

SET(SRCS
  service.cpp
  PythonInterpreter.cpp
  PythonWork.cpp
  PythonUtils.cpp
  util/fileutils.cpp
  Definition.cpp
  DataMapping.cpp
  BuiltIns.cpp
  ${EXT_SRCS}
  ${OS_SRCS}
)
SET(HDRS PythonInterpreter.hh)
SET(LIBS ${pythonDylibName})

BPAddCppService()

SET(stdlibSource "${extLibDir}")
SET(stdlibDest "${OUTPUT_DIR}")

IF (UNIX)
  SET(RM_CMD rm -rf)
ELSE ()
  IF (WIN32) 
    SET(RM_CMD ${CMAKE_COMMAND} -E remove_directory)
  ELSE ()
    SET(RM_CMD "del /f /q")
  ENDIF ()
ENDIF ()

# handle the .lib .dll dichotomy
IF (WIN32)
  SET (pythonLibToShip_debug ${pythonDylib_debug})
  SET (pythonLibToShip_release ${pythonDylib_release})
  STRING(REGEX REPLACE "lib$" "dll" pythonLibToShip_debug ${pythonDylib_debug})
  STRING(REGEX REPLACE "lib$" "dll" pythonLibToShip_release ${pythonDylib_release})
ELSE()
  SET (pythonLibToShip ${pythonDylib})
ENDIF ()

#IF(WIN32)
#  MESSAGE("python library to link (debug):   ${pythonDylib_debug}" )
#  MESSAGE("python library to link (release): ${pythonDylib_release}" )
#  MESSAGE("library to ship (debug):          ${pythonLibToShip_debug}")
#  MESSAGE("library to ship (release):        ${pythonLibToShip_release}")
#ELSE(WIN32)
#  MESSAGE("python library to link: ${pythonDylib}" )
#  MESSAGE("library to ship:        ${pythonLibToShip}")
#ENDIF(WIN32)

ADD_CUSTOM_COMMAND(
  TARGET ${SERVICE_NAME} POST_BUILD
  COMMAND ${RM_CMD} \"${stdlibDest}\"
  COMMAND ${CMAKE_COMMAND} -E echo cp -r
          \"${stdlibSource}\"
          \"${stdlibDest}\"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          \"${stdlibSource}\"
          \"${stdlibDest}\"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          \"${CMAKE_CURRENT_SOURCE_DIR}/python/browserplus.py\"
          \"${stdlibDest}\"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt\"
          \"${OUTPUT_DIR}\"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          \"${pythonLibToShip}\"
          \"${OUTPUT_DIR}\"
)

IF (APPLE) 
  # repeat after me:  I love dylibs
  # this post-build step will help the service find our embedded dynamic python
  # dylib, and will help THAT guy find stdlib extensions (.bundles in ext/)
  SET(libpythonBefore "libpython2.6.dylib")
  SET(libpythonAfter "libpython2.6_bp.dylib")
  ADD_CUSTOM_COMMAND(
    TARGET ${SERVICE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy \"${OUTPUT_DIR}/${libpythonBefore}\" \"${OUTPUT_DIR}/${libpythonAfter}\"  
    COMMAND ${CMAKE_COMMAND} -E remove \"${OUTPUT_DIR}/${libpythonBefore}\"
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup.sh | tr -d '\r' > ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup_unix.sh
    COMMAND chmod 777 ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup_unix.sh
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup_unix.sh ${loc}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup_unix.sh ${loc}
    COMMAND ${CMAKE_COMMAND} -E remove \"${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup_unix.sh\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${loc}\" \"${OUTPUT_DIR}\"
  )
ENDIF ()

## copy in manifest
#GET_FILENAME_COMPONENT(ServiceLibrary "${loc}" NAME)
#IF (WIN32)
#  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
#                 "${OUTPUT_DIR_debug}/manifest.json") 
#  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
#                 "${OUTPUT_DIR_release}/manifest.json") 
#ELSE (WIN32)
#  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
#                 "${OUTPUT_DIR}/manifest.json") 
#ENDIF (WIN32)
#ADD_CUSTOM_COMMAND(TARGET ${SERVICE_NAME} POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                           \"${loc}\" \"${OUTPUT_DIR}\")

