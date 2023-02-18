:: Creates installer and zipped executable of the Compiler for x86 and x64
@ECHO OFF

:: x64
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
ECHO Configuring CMake for x64 ...
cmake -B ../build/msvc_x64 -DCMAKE_BUILD_TYPE=Release -G "Ninja" ..
ECHO Building Interpreter for x64 ...
cmake --build ../build/msvc_x64 --config Release --target Cellox

:: x86
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
ECHO Configuring CMake for x86 ...
cmake -B ../build/msvc_x86 -DCMAKE_BUILD_TYPE=Release -G "Ninja" ..
ECHO Building Interpreter for x86 ...
cmake --build ../build/msvc_x86 --config Release --target Cellox

:: x64_x86 (AMD)
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsamd64_x86.bat"
ECHO Configuring CMake for x64_x86 ...
cmake -B ../build/x64_x86 -DCMAKE_BUILD_TYPE=Release -G "Ninja" ..
ECHO Building Interpreter for x64_x86 ...
cmake --build ../build/x64_x86 --config Release --target Cellox

:: x86_x64 (AMD)
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsx86_amd64.bat"
ECHO Configuring CMake for x86_x64 ...
cmake -B ../build/x86_x64 -DCMAKE_BUILD_TYPE=Release -G "Ninja" ..
ECHO Building Interpreter for x86_x64 ...
cmake --build ../build/x86_x64 --config Release --target Cellox

:: Zipping binaries and creating installer using NSIS (nullsoft scriptable install system) 
:: (https://sourceforge.net/projects/nsis/) for Windows for x86 and x64

:: x64
ECHO Zipping cellox x64 ...
IF NOT EXIST ..\build\msvc_x64\src (
    ECHO Can not find x64 Build folder ...
    EXIT
)
cd ..\build\msvc_x64\src
cpack -G ZIP --config ../CPackConfig.cmake
ECHO Creating installer - x64 using NSIS ...
cpack -G NSIS64 --config ../CPackConfig.cmake

:: x86
ECHO Zipping cellox x86 ...
IF NOT EXIST ..\..\msvc_x86\src (
    ECHO Can not find x64 Build folder ...
    EXIT
)
cd ..\..\msvc_x86\src
cpack -G ZIP --config ../CPackConfig.cmake
ECHO Creating installer - x86 using NSIS ...
cpack -G NSIS --config ../CPackConfig.cmake

:: x64_86
ECHO Zipping cellox x64_x86 ...
IF NOT EXIST ..\..\x64_x86\src (
    ECHO Can not find x64_x86 Build folder ...
    EXIT
)
cd ..\..\x64_x86 \src
cpack -G ZIP --config ../CPackConfig.cmake
ECHO Creating installer - x64_x86  using NSIS ...
cpack -G NSIS --config ../CPackConfig.cmake

:: x86_x64
ECHO Zipping cellox x86_x64 ...
IF NOT EXIST ..\..\x86_x64\src (
    ECHO Can not find x86_x64 Build folder ...
    EXIT
)
cd ..\..\x86_x64\src
cpack -G ZIP --config ../CPackConfig.cmake
ECHO Creating installer - x86_x64 using NSIS ...
cpack -G NSIS --config ../CPackConfig.cmake

cd ..\..\..\scripts