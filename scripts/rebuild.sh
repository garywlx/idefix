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
	make -j4
#   cp ../specs/FIXFXCM10.xml .
#	cp ../specs/fxcm.cfg .
fi