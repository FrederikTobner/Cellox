:: Tests the Interpreter
@ECHO OFF
ECHO Configuring CMake ...
cmake -B ../build -S .. --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17" ..
ECHO Building Tests ...
cmake --build ../build --config Debug
IF NOT EXIST ..\build\test (
    ECHO No Test build folder generated ...
    EXIT
)
cd ..\build\test
ECHO Executing tests
:: Executing tests single threaded because we redirect the standard output to a string and can't therefor execute the tests in parallel
ctest -j 1 -C Debug
cd ..\..\scripts