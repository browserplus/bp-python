#####
# A CMake build file for the PythonInterpreter service
#
# Author: Gary MacDonald
# (c) Yahoo 2010, all rights reserved
#####

# require cmake 2.8.1 or higher
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1 FATAL_ERROR)

# pull in build configuration
INCLUDE(BuildConfigs.cmake)

SET (serviceName PythonInterpreter)

PROJECT(${serviceName}Service)

IF (WIN32)
  SET (outputDir "${CMAKE_CURRENT_BINARY_DIR}/${serviceName}$(OutDir)")
  SET (outputDir_debug "${CMAKE_CURRENT_BINARY_DIR}/${serviceName}Debug")
  SET (outputDir_release "${CMAKE_CURRENT_BINARY_DIR}/${serviceName}Release")
ELSE (WIN32)
  SET (outputDir "${CMAKE_CURRENT_BINARY_DIR}/${serviceName}")
ENDIF (WIN32)

# build up the "platform identifier"
IF (APPLE)
   execute_process(COMMAND uname -p
                   OUTPUT_VARIABLE arch
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

   execute_process(COMMAND uname -r
                   OUTPUT_VARIABLE ver
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

   execute_process(COMMAND uname -s
                   OUTPUT_VARIABLE sysUpper
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

   string(TOLOWER ${sysUpper} sys)

   SET( platIdent "${arch}-${sys}${ver}" )

   MESSAGE("platform: ${platIdent}")
ELSE ()
   # hard coded for now!
   SET( platIdent "i386-mswin32_90" )
ENDIF ()

# looser syntax (that's my patch!)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

# default to a debug build.  This gets us symbols by default under
# the cmake makefile generator
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Debug")
ENDIF ()

SET(extDir "${CMAKE_CURRENT_SOURCE_DIR}/../external")
IF(WIN32)
  SET(extLibDir "${extDir}/dist/lib/$(OutDir)")
  SET(extLibDir_debug "${extDir}/dist/lib/Debug")
  SET(extLibDir_release "${extDir}/dist/lib/Release")
ELSE(WIN32)
  #SET(extLibDir "${extDir}/dist/lib/${CMAKE_BUILD_TYPE}")
  SET(extLibDir "${extDir}/dist/lib")
ENDIF(WIN32)
SET(extIncDir "${extDir}/dist/include/python26")
SET(CPPTOOLS_DIR "${extDir}/bp-service-tools/CppTools/src")

INCLUDE_DIRECTORIES(
  "${extIncDir}/${platIdent}"
  "${extIncDir}"
  "${extDir}/bpsdk/include"
  "${CPPTOOLS_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

SET(pythonLibPath ${extLibDir})
LINK_DIRECTORIES(${pythonLibPath})

SET(EXT_SRCS
  "${CPPTOOLS_DIR}/bptypeutil.cpp"
  "${CPPTOOLS_DIR}/bpservicedescription.cpp"
  "${CPPTOOLS_DIR}/bpserviceversion.cpp"
  "${CPPTOOLS_DIR}/bpurlutil.cpp"
)

SET(SRCS
  service.cpp
  PythonInterpreter.cpp
  PythonWork.cpp
  PythonUtils.cpp
  util/fileutils.cpp
  Definition.cpp
  DataMapping.cpp
  BuiltIns.cpp
)

# os specific srcs
IF (WIN32)
   FILE(GLOB OS_SRCS "util/*_Windows.cpp")
ELSE()
   FILE(GLOB OS_SRCS "util/*_Darwin.cpp" "util/*_UNIX.cpp")
ENDIF ()

FILE(GLOB_RECURSE HDRS "*.hh")

ADD_LIBRARY(${serviceName} MODULE ${EXT_SRCS} ${OS_SRCS} ${SRCS} ${HDRS})

# find the path to the python library we'll link
MESSAGE("looking in ${pythonLibPath}")
IF(WIN32)
  FIND_LIBRARY(pythonDylib_debug NAMES python2.6_d python26_d
               PATHS ${extLibDir_debug}
               NO_DEFAULT_PATH
               NO_SYSTEM_ENVIRONMENT_PATH)
  GET_FILENAME_COMPONENT(pythonDylibName_debug ${pythonDylib_debug} NAME)
  FIND_LIBRARY(pythonDylib_release NAMES python2.6 python26
               PATHS ${extLibDir_release}
               NO_DEFAULT_PATH
               NO_SYSTEM_ENVIRONMENT_PATH)
  GET_FILENAME_COMPONENT(pythonDylibName_release ${pythonDylib_release} NAME)

  TARGET_LINK_LIBRARIES(${serviceName}
	                debug ${pythonDylibName_debug}
	                optimized ${pythonDylibName_release})
ELSE(WIN32)
  FIND_LIBRARY(pythonDylib NAMES python2.6 python26
               PATHS ${pythonLibPath}
               NO_DEFAULT_PATH
               NO_SYSTEM_ENVIRONMENT_PATH)
  GET_FILENAME_COMPONENT(pythonDylibName ${pythonDylib} NAME)

  TARGET_LINK_LIBRARIES(${serviceName} ${pythonDylibName})
ENDIF(WIN32)

# create service directory
IF (WIN32)
  FILE(MAKE_DIRECTORY ${outputDir_debug})
  FILE(MAKE_DIRECTORY ${outputDir_release})
ELSE (WIN32)
FILE(MAKE_DIRECTORY ${outputDir})
ENDIF (WIN32)

IF (APPLE)
#  ADD_CUSTOM_COMMAND(TARGET ${serviceName} POST_BUILD
#                     COMMAND strip -x \"${outputDir}/${ServiceLibrary}\")
ENDIF (APPLE)

# get location of output service library
GET_TARGET_PROPERTY(loc ${serviceName} LOCATION)

# handle the .lib .dll dichotomy
IF (WIN32)
  SET (pythonLibToShip_debug ${pythonDylib_debug})
  SET (pythonLibToShip_release ${pythonDylib_release})
  STRING(REGEX REPLACE "lib$" "dll" pythonLibToShip_debug ${pythonDylib_debug})
  STRING(REGEX REPLACE "lib$" "dll" pythonLibToShip_release ${pythonDylib_release})
ELSE()
  SET (pythonLibToShip ${pythonDylib})
ENDIF ()

IF(WIN32)
  MESSAGE("python library to link (debug):   ${pythonDylib_debug}" )
  MESSAGE("python library to link (release): ${pythonDylib_release}" )
  MESSAGE("library to ship (debug):          ${pythonLibToShip_debug}")
  MESSAGE("library to ship (release):        ${pythonLibToShip_release}")
ELSE(WIN32)
  MESSAGE("python library to link: ${pythonDylib}" )
  MESSAGE("library to ship:        ${pythonLibToShip}")
ENDIF(WIN32)

SET(stdlibSource "${extLibDir}")
SET(stdlibDest "${outputDir}")

IF(WIN32)
  ADD_CUSTOM_COMMAND(
    TARGET ${serviceName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        \"${stdlibSource}\" \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${CMAKE_CURRENT_SOURCE_DIR}/python/browserplus.py\"
            \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt\"
            \"${outputDir}\"
    )
ELSE(WIN32)
  ADD_CUSTOM_COMMAND(
    TARGET ${serviceName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        \"${stdlibSource}\" \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${CMAKE_CURRENT_SOURCE_DIR}/python/browserplus.py\"
            \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt\"
            \"${outputDir}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${pythonLibToShip}\"
            \"${outputDir}\"
  )
ENDIF(WIN32)

IF (APPLE) 
  # repeat after me:  I love dylibs
  # this post-build step will help the service find our embedded dynamic python
  # dylib, and will help THAT guy find stdlib extensions (.bundles in ext/)
  SET(libpythonBefore "libpython2.6.dylib")
  SET(libpythonAfter "libpython2.6_bp.dylib")

  ADD_CUSTOM_COMMAND(
      TARGET ${serviceName} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy \"${outputDir}/${libpythonBefore}\" \"${outputDir}/${libpythonAfter}\"  
      COMMAND ${CMAKE_COMMAND} -E remove \"${outputDir}/${libpythonBefore}\"
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup.sh ${loc}
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup.sh ${loc}
      )
ENDIF ()

# copy in manifest
GET_FILENAME_COMPONENT(ServiceLibrary "${loc}" NAME)
IF (WIN32)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
                 "${outputDir_debug}/manifest.json") 
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
                 "${outputDir_release}/manifest.json") 
ELSE (WIN32)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
                 "${outputDir}/manifest.json") 
ENDIF (WIN32)
ADD_CUSTOM_COMMAND(TARGET ${serviceName} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           \"${loc}\" \"${outputDir}\")
