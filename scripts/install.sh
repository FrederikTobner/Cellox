#!/bin/bash
cd ../build/src
echo "Installing Cellox ..."
cmake -B ./build -DCMAKE_BUILD_TYPE=Release
cmake --build ./build --config Release --target Cellox
cmake --install . --config Release