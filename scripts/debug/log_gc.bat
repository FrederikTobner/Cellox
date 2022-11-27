@ECHO OFF
ECHO Configuring CMake ...
cmake -B ../../build -DCMAKE_BUILD_TYPE=Debug -DDEBUG_LOG_GARBAGE_COLLECTOIN=ON -G Ninja -DCMAKE_C_COMPILER=C:/MinGW64/bin/gcc.exe ../..
ECHO Building Interpreter ...
cmake --build ../../build --config Debug --target Cellox