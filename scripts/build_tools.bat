@ECHO OFF
set buildType=%1
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
ECHO Configuring CMake ...
cmake -B ../build -DCMAKE_BUILD_TYPE=%buildType% -DCLX_BUILD_TOOLS=ON -G "Ninja" ..
ECHO Building Compiler ...
cmake --build ../build --config %buildType% --target CelloxBenchmarks CelloxDisassembler