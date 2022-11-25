ECHO Building x86 version of the interpreter under windows ...
cd ..
cmake -B ..\build\win_x86 -DCMAKE_BUILD_TYPE=RELEASE -A Win32 -G "Visual Studio 17 2022"
cmake --build ..\build\win_x86 --config RELEASE --target Cellox