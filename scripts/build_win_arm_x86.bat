ECHO Building x86 Arm version of the interpreter under windows ...
cd ..
cmake -B .\build\arm_x86 -DCMAKE_BUILD_TYPE=RELEASE -A ARM -G "Visual Studio 17 2022"
cmake --build .\build\arm_x86 --config RELEASE --target Cellox