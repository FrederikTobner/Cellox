# Creates installer and zipped executable of the Interpreter for x86 and x64
# Configuring CMake and building for x86 and x64 using msvc
echo "Configuring CMake for x64 ..."
cmake -B ../build/msvc_x64 -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17" ..
echo "Building Interpreter for x64 ..."
cmake --build ../build/msvc_x64 --config Release --target Cellox
echo "Configuring CMake for x86 ..."
cmake -B ../build/msvc_x86 -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17" -A Win32 ..
echo "Building Interpreter for x86 ..."
cmake --build ../build/msvc_x86 --config Release --target Cellox
# Zipping binaries and creating installer using NSIS (nullsoft scriptable install system) 
# (https://sourceforge.net/projects/nsis/) for Windows for x86 and x64
echo "Zipping cellox x64 ..."
if [ -d "../build/msvc_x64/src" ] 
    then
    cd ../build/msvc_x64/src
    cpack -G ZIP --config ../CPackConfig.cmake
    echo "Creating installer - x64 using NSIS ..."
    cpack -G NSIS64 --config ../CPackConfig.cmake
    echo "Zipping cellox x86 ..."
    if [ -d "../../msvc_x86/src"]
        then
        cd ../../msvc_x86/src
        cpack -G ZIP --config ../CPackConfig.cmake
        echo "Creating installer - x86 using NSIS ..."
        cpack -G NSIS --config ../CPackConfig.cmake
        cd ../../../scripts
        else
        echo "Can not find x64 Build folder ..."
        exit 70
    fi
    else
    echo "Can not find x86 Build folder ..."
    exit 70
fi