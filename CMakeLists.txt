cmake_minimum_required(VERSION 2.8)
project(OSPRayTinyLoader)
#
# include host specification file
cmake_host_system_information(RESULT SYSNAME QUERY HOSTNAME)
message(STATUS "trying to include ${SYSNAME}.cmake configuration file")
include(cmake/config-site/${SYSNAME}.cmake OPTIONAL RESULT_VARIABLE CONFIGFOUND)
if (NOT CONFIGFOUND)
  message(STATUS "failed to load the configuration file")
  message(STATUS "helper message:
    To build the program sucessfully, you might need to provide following paths to your cmake
    *) path to the TBBConfig.cmake of your standalone TBB library as '-DTBB_DIR=<..>'
    *) path to the root directory of your standalone TBB library as '-DTBB_ROOT=<..>'
  ")
endif()
#
# load required libraries
INCLUDE(cmake/common.cmake)
#
# load projects
SET(projdir ${CMAKE_CURRENT_LIST_DIR}/projects)
FILE(GLOB children RELATIVE ${projdir} ${projdir}/*)
FOREACH(child ${children})
  IF(IS_DIRECTORY ${projdir}/${child})
      add_subdirectory(${projdir}/${child})
  ENDIF()
ENDFOREACH()
