#!/bin/bash
# Generates documentation using doxyxgen (https://www.doxygen.nl/)
if [ -d "../src" ]
then
cd ../src
echo "Generating Documentation ..."
doxygen
echo "Moving generated content out of the html folder ..."
cp -avr ../docs/html ../docs
echo "Removing html folder ..."
rm -rf ../docs/html
else
echo "Could not find source folder"
fi