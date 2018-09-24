#!/bin/bash
# Remove build directory and rebuild
cd ..
if [ -d build/ ]; then
	rm -rf build/
	mkdir build
fi
if [ -d build/ ]; then
	cd build
	cmake ..
	make
fi
