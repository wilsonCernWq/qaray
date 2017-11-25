cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0054 OLD)
project(QARAY)
#
# options
OPTION(EMABLE_SHARED "Enable shared Library" OFF)
if(EMABLE_SHARED) 
  set(BUILD_SHARED_LIBS ON)
else(EMABLE_SHARED) 
  set(BUILD_SHARED_LIBS OFF)
endif(EMABLE_SHARED)
#
# include host specification file
cmake_host_system_information(RESULT SYSNAME QUERY HOSTNAME)
message(STATUS "Trying to include ${SYSNAME}.cmake configuration file")
include(cmake/config-site/${SYSNAME}.cmake OPTIONAL RESULT_VARIABLE CONFIGFOUND)
if (NOT CONFIGFOUND)
  message(STATUS "Failed to load the configuration file")
  message(STATUS "Helper message:\n
    To build the program sucessfully, you might need to provide following paths to your cmake
    *) path to the root directory of your standalone TBB library as '-DTBB_ROOT=<..>'       
  ")
endif()
#
# load required libraries
include(cmake/common.cmake)
#
# load projects
add_subdirectory(src)
#
# deploy mesh
file(GLOB OBJS     ${CMAKE_CURRENT_LIST_DIR}/assets/*.obj)
file(GLOB MTLS     ${CMAKE_CURRENT_LIST_DIR}/assets/*.mtl)
file(GLOB TEXTURES ${CMAKE_CURRENT_LIST_DIR}/assets/*.png)
set(ASSETS ${TEXTURES} ${MTLS} ${OBJS})
if(EXECUTABLE_OUTPUT_PATH)
  DeployRepo("${ASSETS}" "${EXECUTABLE_OUTPUT_PATH}")
else()
  DeployRepo("${ASSETS}" "${CMAKE_BINARY_DIR}")
endif()
