#!/bin/bash


writefile=$1
writestr=$2

cdir=$( dirname "$writefile" )


if [ "$#" -eq "2" ]
then
	mkdir -p "$cdir"
	touch "$writefile"
	echo "$writestr" > "$writefile"	
	exit 0
else
	echo "ERROR: Invalid Number of Arguments."
	echo "Total number of arugemts should be 2."
	echo "The order of the arguments should be:"
	echo "  1) Path to the file."
	echo "  2) String to be written in the specified file."
	exit 1
fi
