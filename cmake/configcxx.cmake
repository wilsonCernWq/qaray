#
#--- Configures compiler for C++11
#
ADD_DEFINITIONS(-std=c++11)
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
