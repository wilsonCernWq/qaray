#
#--- Configures compiler for C++11
#
ADD_DEFINITIONS(-std=c++11)
if(UNIX)
  if(NOT APPLE)
    message(STATUS "Using GCC Compiler.")
  else()
    message(STATUS "Using CLANG Intel Compiler. (Untested)")
    set(CMAKE_MACOSX_RPATH 1)
  endif()
else()
  message(STATUS "Using MSVC Intel Compiler.")
endif()
