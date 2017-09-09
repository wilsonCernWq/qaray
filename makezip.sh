#!/bin/bash

N=${1}
if [[ -z "${1}" ]]; then
    echo "Tell me which project you are trying to compress"
    read N
fi

zip -r prj${N}.zip \
    cmake/ \
    CMakeLists.txt \
    external/cyCodeBase \
    external/lodepng \
    external/tinyxml \
    projects/Prj${N}

echo "Now Testing"
echo "A" | unzip prj${N}.zip -d test
cd test
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j || exit
cd ../..
rm -fr test/
