#!/bin/bash
# Remove build directory and rebuild
cd ..
rm -rf build/
mkdir build
cd build
cmake ..
make
