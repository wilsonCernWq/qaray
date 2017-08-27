cmake_minimum_required(VERSION 2.8)
project(OSPRayTinyLoader)
#
# include host specification file
cmake_host_system_information(RESULT SYSNAME QUERY HOSTNAME)
message(STATUS "trying to include ${SYSNAME}.cmake configuration file")
include(cmake/config-site/${SYSNAME}.cmake OPTIONAL RESULT_VARIABLE CONFIGFOUND)
if (NOT CONFIGFOUND)
  message(STATUS "failed to load the configuration file")
  # message(STATUS "helper message:
  #   To build the program sucessfully, you might need to provide following paths to your cmake
  #   *) path to your osprayConfig.cmake included in the OSPRay releases to CMake as '-Dospray_DIR=<..>'
  #   *) path to the embreeConfig.cmake from the Embree releases similarly as '-Dembree_DIR=<..>'
  #   *) path to the root directory of your standalone TBB library as '-DTBB_ROOT=<..>'
  #   *) path to the root directory of your standalone ISPC executable as '-DISPC_EXECUTABLE=<..>'
  # ")
endif()
#
# load required libraries
include(cmake/common.cmake)
find_package(OpenMP)
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
#
# load projects
SET(projdir ${CMAKE_CURRENT_LIST_DIR}/projects)
FILE(GLOB children RELATIVE ${projdir} ${projdir}/*)
FOREACH(child ${children})
  IF(IS_DIRECTORY ${projdir}/${child})
      add_subdirectory(${projdir}/${child})
  ENDIF()
ENDFOREACH()
