#
#--- Configures compiler for C++11
#
if(UNIX)
  if(NOT APPLE)
    # Enable c++11 for GCC
    message(STATUS "Using GCC Compiler.")
    set(CMAKE_C_COMPILER /opt/intel/bin/icc)
    set(CMAKE_CXXXXXXXX_COMPILER /opt/intel/bin/icc)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    # Clang in OSX supports partially c++11 through extensions
    message(STATUS "Using CLANG Intel Compiler. (Untested)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-extensions")
  endif()
else()
  # MSVC12 supports c++11 natively
  message(STATUS "Using MSVC Intel Compiler.")
endif()
