:: Tests the Interpreter
@ECHO OFF
CALL build_tests.bat Debug
IF NOT EXIST ..\build\test (
    ECHO No Test build folder generated ...
    EXIT
)
cd ..\build\test
ECHO Executing tests
:: Executing tests single threaded because we redirect the standard output to a string and can't therefor execute the tests in parallel
ctest -j 1 -C Debug
cd ..\..\scripts