# Tests the Interpreter
echo "Configuring CMake ..."
cmake -B ../build -S .. --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE -DCMAKE_BUILD_TYPE=Debug -G Ninja -DCMAKE_C_COMPILER=/usr/local/bin/clang-16 -DCMAKE_CXX_COMPILER:FILEPATH=/usr/local/bin/clang++ ..
echo "Building Tests ..."
cmake --build ../build --config Debug
if [ -d "../build/test" ]
then
    cd ../build/test
    echo "Executing tests ..."
    # Executing tests single threaded because we redirect the standard output to a string and can't therefor execute the tests in parallel
    ctest -j 1 -C Debug
    cd ../../scripts
else
    echo "No Test build folder generated ..."
    exit 70
fi