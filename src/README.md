# Tested System

* CentOS Linux release 7.2.1511 (Core)
* MacOS Sierra version 10.12.6

# Tested Compiler

* gcc (GCC) 6.3.0 / 7.1.0 
* Apple LLVM version 8.1.0 (clang-802.0.42)
* Intel ICC 17 (raypacket project is not compiled with intel ICC)

# Optional library

* Multi-Threading (Intel TBB / OpenMP)
* MPI Parallelism
* GLM

# How to compile the code

If you want to use TBB's dynamic task scheduling system for
multi-threading, but don't have TBB installed, you need to download
TBB from [here](https://github.com/01org/tbb/releases).
After extracting the library into a folder, you should pass the
directory to CMake using `TBB_ROOT` variable.

If you are building the program for local usage with OpenGL, you
will want to enable the `ENABLE_GUI` flag for CMake. Otherwise
you should disable it.

If you are building the program for cluster, you can enable the
`ENABLE_MPI` flag for CMake

```
mkdir build
cd build
ccmake .. \
       -DCMAKE_BUILD_TYPE=Release \
       -DTBB_ROOT=</path/to/your/tbb/folder> 
make -j8
```

# How to run the the project

```
cd build
./qaray </path/to/the/input>
```