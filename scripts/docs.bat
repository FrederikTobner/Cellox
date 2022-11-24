ECHO Generating Documentation ...
cd ..
doxygen
copy ../docs/html ../docs
rmdir -s ../docs/html