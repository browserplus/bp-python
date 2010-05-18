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

SET (outputDir "${CMAKE_CURRENT_BINARY_DIR}/${serviceName}")

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
SET(extLibDir "${extDir}/${CMAKE_SYSTEM_NAME}/lib/${CMAKE_BUILD_TYPE}")
SET(extIncDir "${extDir}/${CMAKE_SYSTEM_NAME}/include/python26")
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

SET(HDRS PythonInterpreter.hh)
ADD_LIBRARY(${serviceName} MODULE ${EXT_SRCS} ${OS_SRCS} ${SRCS} ${HDRS})

# find the path to the python library we'll link
MESSAGE("looking in ${pythonLibPath}")
FIND_LIBRARY(pythonDylib NAMES python2.6 python26
             PATHS ${pythonLibPath}
             NO_DEFAULT_PATH
             NO_SYSTEM_ENVIRONMENT_PATH)
GET_FILENAME_COMPONENT(pythonDylibName ${pythonDylib} NAME)

TARGET_LINK_LIBRARIES(${serviceName} ${pythonDylibName})

# create service directory
FILE(MAKE_DIRECTORY ${outputDir})

IF (APPLE)
#  ADD_CUSTOM_COMMAND(TARGET ${serviceName} POST_BUILD
#                     COMMAND strip -x \"${outputDir}/${ServiceLibrary}\")
ENDIF (APPLE)



SET(stdlibDest "${outputDir}/stdlib")
SET(stdlibSource "${extLibDir}/python2.6")

SET(extDest "${outputDir}/ext")
FILE(GLOB extSource "${extLibDir}/python/2.6.5/i386-*")

IF (APPLE)
    SET(RM_CMD rm -rf)
ELSE ()
    SET(RM_CMD "del /f /q")
ENDIF ()

GET_FILENAME_COMPONENT( pythonDylibName ${pythonDylib} NAME )

# get location of output service library
GET_TARGET_PROPERTY(loc ${serviceName} LOCATION)

# handle the .lib .dll dichotomy
SET (pythonLibToShip ${pythonDylib})
IF (WIN32) 
  STRING(REGEX REPLACE "lib$" "dll" pythonLibToShipA ${pythonDylib})
  STRING(REGEX REPLACE "/lib/" "/bin/" pythonLibToShip ${pythonLibToShipA})
ENDIF () 

MESSAGE("python library to link:  ${pythonDylib}" )
MESSAGE("library to ship:       ${pythonLibToShip}")
MESSAGE("python library to link:  ${pythonDylibA}" )
MESSAGE("library to ship:       ${pythonLibToShipA}")

#ADD_CUSTOM_COMMAND(
#    TARGET ${serviceName} POST_BUILD
#    COMMAND ${RM_CMD} \"${stdlibDest}\"
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#                        \"${stdlibSource}\" \"${stdlibDest}\"
#    COMMAND ${CMAKE_COMMAND} -E echo
#                        \"${stdlibSource}\" \"${stdlibDest}\"
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            \"${CMAKE_CURRENT_SOURCE_DIR}/python/browserplus.rb\"
#			\"${stdlibDest}\"
#    COMMAND ${RM_CMD} \"${extDest}\"
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#                        \"${extSource}\" \"${extDest}\"
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt\"
#            \"${outputDir}\"
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            \"${pythonLibToShip}\"
#            \"${outputDir}\"
#)

#IF (APPLE)  
#  # repeat after me:  I love dylibs
#  # this post-build step will help the service find our embedded dynamic python
#  # dylib, and will help THAT guy find stdlib extensions (.bundles in ext/)
#  SET(libpythonBefore "libpython.dylib")
#  SET(libpythonAfter "libpython_bp.dylib")
#
#  ADD_CUSTOM_COMMAND(
#      TARGET ${serviceName} POST_BUILD
#      COMMAND ${CMAKE_COMMAND} -E copy \"${outputDir}/${libpythonBefore}\" \"${outputDir}/${libpythonAfter}\"   
#      COMMAND ${CMAKE_COMMAND} -E remove \"${outputDir}/${libpythonBefore}\"
#      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup.sh ${loc}
#      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libpython_fixup.sh ${loc}
#      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ext_fixup.sh \"${extDest}\"
#      )
#ENDIF ()

# copy in manifest
GET_FILENAME_COMPONENT(ServiceLibrary "${loc}" NAME)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
               "${outputDir}/manifest.json")  
ADD_CUSTOM_COMMAND(TARGET ${serviceName} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           \"${loc}\" \"${outputDir}\")
