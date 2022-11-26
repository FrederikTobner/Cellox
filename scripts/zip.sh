#!/bin/bash
cd ../build/src
echo "Zipping cellox ..."
cpack -G ZIP --config ../CPackConfig.cmake