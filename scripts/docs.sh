#!/bin/bash
# Generates documentation using doxyxgen (https://www.doxygen.nl/)
if [ -d "../src" ]
then
    cd ../src
    echo "Generating Documentation ..."
    doxygen
    echo "Moving generated content out of the html folder ..."
    if [ -d "../docs" ]
    then
        echo "No docs folder generated"
        exit 70
    else
        if [ -d "../docs/html" ]
        then
            cp -avr ../docs/html ../docs
            echo "Removing html folder ..."
            rm -rf ../docs/html
        else
            echo "No html folder inside the docs foulder found"
        fi
    fi
else
    echo "Could not find source folder"
fi