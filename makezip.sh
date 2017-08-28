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
