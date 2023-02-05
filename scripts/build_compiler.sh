echo "Configuring CMake ..."
cmake -B ../build -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_C_COMPILER=/usr/local/bin/clang ..
echo "Building Compiler ..."
cmake --build ../build --config Release --target Cellox