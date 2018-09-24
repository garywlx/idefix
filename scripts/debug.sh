#!/bin/bash
# Remove build directory and rebuild in debug mode
cd ..
if [ -d build/ ]; then
	rm -rf build/
	mkdir build
fi
if [ -d build/ ]; then
	cd build
	cmake -DCMAKE_BUILD_TYPE=Debug ..
	make
else
	echo "build directory does not exist."
	exit
fi
