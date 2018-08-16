#!/bin/bash
## tick data download from fxcm public api

#title			:tickdatadl
#description	:tick data download per week from fxcm public api
#author			:Arne Gockeln <arne@gockeln.com>
#version		:0.1
#notes			:requires tr and dos2unix
#=================================================================================

if [ $# -lt 3 ]; then
	echo "This script downloads tick data csv files from FXCM public servers."
	echo "Usage:"
	echo "	$0 symbol year week"
	exit 1
fi

SYMBOL=$1
YEAR=$2
WEEK=$3

DESTFILE="$SYMBOL-$YEAR-$WEEK.csv"

# download path
DLFILE="https://tickdata.fxcorporate.com/$SYMBOL/$YEAR/$WEEK.csv.gz"

if [ -d /tmp ]; then
	# download file to tickdata.csv.gz
	echo "Download..."
	curl $DLFILE > /tmp/tickdata.csv.gz

	if [ -f /tmp/tickdata.csv.gz ]; then
		# gunzip
		echo "Extract..."
		gunzip /tmp/tickdata.csv.gz
	fi
	if [ -f /tmp/tickdata.csv ]; then
		# replace 0byte
		echo "Convert..."
		tr < /tmp/tickdata.csv -d '\000' > /tmp/tickdata.csv.out
		# convert to unix
		dos2unix /tmp/tickdata.csv.out
		# move to destination
		echo "Move to $DESTFILE"
		mv /tmp/tickdata.csv.out $DESTFILE
		# clean up
		rm /tmp/tickdata.csv
		echo "Done."
	fi
fi