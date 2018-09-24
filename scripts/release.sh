#!/bin/bash
# builds a release version

# leave scripts directory
cd ..
# check if build directory exists
if [ -d build ]; then
	# remove build dir
	rm -rf ./build
	# make new build dir
	mkdir ./build
fi

if [ -d build ]; then
	# enter build directory
	cd build

	# call cmake with build type release
	cmake -DCMAKE_BUILD_TYPE=Release .. 

	# compile
	make
else
	echo "build directory does not exist."
	exit
fi