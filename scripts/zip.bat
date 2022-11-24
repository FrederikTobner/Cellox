cd ../build/src
cpack -G ZIP --config ..\CPackConfig.cmake
:: Create installer using NSIS (nullsoft scriptable install system) (https://sourceforge.net/projects/nsis/) for Windows