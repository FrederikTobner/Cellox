ECHO Building x64 Arm version of the interpreter under windows ...
cd ..
cmake -B .\build\arm_x64 -DCMAKE_BUILD_TYPE=RELEASE -A ARM64 -G "Visual Studio 17 2022"
cmake --build .\build\arm_x64 --config RELEASE --target Cellox