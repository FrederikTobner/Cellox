echo "Configuring CMake ..."
cmake -B ../build -S .. --no-warn-unused-cli -DCLX__BUILD_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_C_COMPILER=/usr/local/bin/clang-16 -DCMAKE_CXX_COMPILER:FILEPATH=/usr/local/bin/clang++ ..
echo "Building Tests ..."
cmake --build ../build --config Debug --target CelloxTests