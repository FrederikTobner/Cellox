ECHO Building x64 version of the interpreter under windows ...
cd ..
cmake -B .\build\win_x64 -DCMAKE_BUILD_TYPE=RELEASE -A Win64 -G "Visual Studio 17 2022"
cmake --build .\build\win_x64 --config RELEASE --target Cellox