# Creates zipped executable of the Compiler for x64
# Configuring CMake
echo "Configuring CMake for x64 ..."
cmake -B ../build/linux_x64 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -G Ninja  -DCMAKE_C_COMPILER=/usr/local/bin/clang -A x64 ..
echo "Building Interpreter for x64 ..."
cmake --build ../build/linux_x64 --config Release --target Cellox
echo "Configuring CMake for x86 ..."
cmake -B ../build/linux_x86 -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -G Ninja  -DCMAKE_C_COMPILER=/usr/local/bin/clang -A x86 ..
echo "Building Interpreter for x86 ..."
cmake --build ../build/linux_x86 --config Release --target Cellox
# Zipping binaries
echo "Zipping cellox x64 ..."
if [ -d "../build/linux_x64/src" ] 
then
    cd ../build/linux_x64/src
    cpack -G ZIP --config ../CPackConfig.cmake.    
else
    echo "Can not find x64 linux build folder ..."
    exit 70
fi
echo "Zipping cellox x86 ..."
if [ -d "../build/linux_x86/src" ] 
then
    cd ../build/linux_x86/src
    cpack -G ZIP --config ../CPackConfig.cmake.    
else
    echo "Can not find x86 linux build folder ..."
    exit 70
fi