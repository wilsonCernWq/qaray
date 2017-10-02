# Tested System

* CentOS Linux release 7.2.1511 (Core)
* MacOS Sierra version 10.12.6

# Tested Compiler

* gcc (GCC) 6.3.0 / 7.1.0 
* Apple LLVM version 8.1.0 (clang-802.0.42)
* Intel ICC 17 (raypacket project is not compiled with intel ICC)

# Optional library

* Libsimdpp
* GLM

# How to compile the code

```
mkdir build
cd build
ccmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_LIBSIMDPP=ON -DENABLE_SIMDTYPE_PROJECT=ON
make -j8
```
