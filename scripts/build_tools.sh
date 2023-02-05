echo "Configuring CMake ..."
cmake -B ../build -DCMAKE_BUILD_TYPE=Release -DCLX_BUILD_TOOLS=ON --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_C_COMPILER=/usr/local/bin/clang-16 ..
echo "Building Compiler ..."
cmake --build ../build --config Release --target CelloxBenchmarks CelloxDisassembler