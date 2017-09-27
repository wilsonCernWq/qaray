# Working System

CentOS Linux release 7.2.1511 (Core)
MacOS Sierra version 10.12.6

# Working Compiler

gcc (GCC) 7.1.0 / Apple LLVM version 8.1.0 (clang-802.0.42)

# Additional library

Intel TBB

# How to compile the code

Download TBB from https://github.com/01org/tbb/releases if you dont have TBB installed
Extract the library into a folder

```
mkdir build
cd build
cmake .. -DTBB_ROOT=</path/to/your/tbb/folder>
make -j
```

# How to run the the project

```
cd build
./projects/Prj<N>/raytracer ../projects/Prj<N>/inputs/example_Blinn.xml
```