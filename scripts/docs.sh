#!/bin/bash
echo "Generating Documentation ..."
cd ..
doxygen
cp -avr ../docs/html ../docs
rm -rf ../docs/html