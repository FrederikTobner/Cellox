# Tests the Interpreter
echo "Configuring CMake ..."
cmake -B ../build -S .. --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17" ..
echo "Building Tests ..."
cmake --build ../build --config Debug
if [ -d "../build/test" ]
then
    cd ../build/test
    echo "Executing tests ..."
    ctest -j 1 -C Debug
    cd ../../scripts
else
    echo "No Test build folder generated ..."
    exit 70
fi