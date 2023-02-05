:: Installs Cellox on your system but does not add it to the path
:: Administrator permission is needed to install Cellox
@ECHO OFF
CALL build_compiler.bat Release
ECHO Installing Compiler ...
cmake --install ../build/src --config Release