#!/bin/bash
cd ..
echo "Configuring CMake ..."
cmake -B ./build -DCMAKE_BUILD_TYPE=Release
echo "Building Interpreter ..."
cmake --build ./build --config Release --target Cellox
echo "Zipping cellox ..."
cd build/src
cpack -G ZIP --config ../CPackConfig.cmake