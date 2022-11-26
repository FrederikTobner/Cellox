#!/bin/bash
cd ..
echo "Generating Documentation ..."
doxygen
echo "Moving generated content out of the html folder ..."
cp -avr ../docs/html ../docs
echo "Removing html folder ..."
rm -rf ../docs/html