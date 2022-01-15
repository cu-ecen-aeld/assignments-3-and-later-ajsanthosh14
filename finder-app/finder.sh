#!/bin/bash

##########################################################
#       S22-AESD-Assignment-1: finder.sh                 #
#                                                        #
#       Author: Santhosh, santhosh@colorado.edu          # 
#                                                        # 
##########################################################

#Input arguments
FILESDIR=$1
SEARCHSTR=$2

#checks for the valid number of arguments
if [ "$#" -ne "2" ]
then 
	cat  <<- EOF
	ERROR: Invalid Number of Arguments
	Total number of arguments should be 2.
	The order of the arguments should be:
	  1) Path to a directory on the filesystem.
	  2) String to be searched within the specified filesystem.
	EOF
	exit 1
fi

#checks if the file directory exists
if [ -d "$FILESDIR" ]
then
	NUMFILES=$(find "$FILESDIR" -type f | wc -l)
	NUMLINES=$(grep -R "$SEARCHSTR" "$FILESDIR" | wc -l)
	echo "The number of files are $NUMFILES and the number of matching lines are $NUMLINES"
	exit 0
else
	echo "$FILESDIR: does not exists"
	exit 1
fi



