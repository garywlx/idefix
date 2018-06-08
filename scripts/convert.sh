#!/bin/bash
#title			:convert
#description	:removes nullbytes and dos special characters from fxcm csv files
#author			:Arne Gockeln, www.arnegockeln.com
#version		:0.1
#notes			:requires tr and dos2unix
#=================================================================================

if [ $# -lt 1 ]; then
	echo "This script removes nullbytes and dos special characters from fxcm csv files"
	echo "Usage:"
	echo "	$0 /path/to/file.csv"
	exit 1
fi

INPUT=$1
OUTPUT="${INPUT}_out"
# remove nullbyte
tr < $INPUT -d '\000' > $OUTPUT
# convert from dos 2 unix
dos2unix $OUTPUT
# move output to original file
cat $OUTPUT > $INPUT
# remove output
rm $OUTPUT