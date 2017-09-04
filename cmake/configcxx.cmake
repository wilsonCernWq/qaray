#
#--- Configures compiler for C++11
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
IF(UNIX)
  IF(NOT APPLE)
    MESSAGE(STATUS "Using GCC Compiler.")
  ELSE()
    MESSAGE(STATUS "Using CLANG Intel Compiler. (Untested)")
    SET(CMAKE_MACOSX_RPATH 1)
  ENDIF()
ELSE()
  MESSAGE(STATUS "Using MSVC Intel Compiler.")
ENDIF()
