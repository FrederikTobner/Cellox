#!/bin/bash
echo "Configuring CMake ..."
cmake -B ../build -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_C_COMPILER=/user/local/bin/clang
echo "Building Interpreter ..."
cmake --build ../build --config Debug --target Cellox