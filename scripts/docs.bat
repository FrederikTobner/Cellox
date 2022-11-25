ECHO Generating Documentation ...
cd ..\src
doxygen
copy ..\docs\html ..\docs
rmdir /s ..\docs\html\