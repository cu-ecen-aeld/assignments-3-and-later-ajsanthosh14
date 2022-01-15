#!/bin/bash

filesdir=$1
searchstr=$2

#checks for the unspecified arguments
if [ "$#" -eq "2" ]
then 
	if [ -d "$filesdir" ]
	then
		echo "The number of files are $(find "$filesdir" -type f | wc -l) and the number of matching lines are $(grep -R "$searchstr" "$filesdir" | wc -l)"
		exit 0
	else 
		echo "DIR not found"
		exit 1
	fi
else 
	echo "ERROR: Invalid Number of Arguments."
	echo "Total number of arguments should be 2."
	echo "The order of the arguments should be:"
	echo "  1) Path to a directory on the filesystem."
	echo "  2) String to be searched within the specified filesystem."
	exit 1

fi


