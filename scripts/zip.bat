ECHO Zipping cellox and creating installer using NSIS ...
cd ../build/src
:: Zip cellox executable
cpack -G ZIP --config ..\CPackConfig.cmake
:: Create an installer using NSIS (nullsoft scriptable install system) (https://sourceforge.net/projects/nsis/) for Windows
cpack -G NSIS64 --config ..\CPackConfig.cmake