#!/bin/bash

########################################################
#                                                      #
#	S22-AESD-Assignment-1 writer.sh                #
#                                                      #
#	Author:  Santhosh, santhosh@ccolorado.edu      #
#                                                      #
########################################################

#Input arguments
WRITEFILE=$1
WRITESTR=$2

#extracts only the directory 
CDIR=$( dirname "$WRITEFILE" )

#checks for valid number of arguments
if [ "$#" -ne "2" ]
then
	cat <<- EOF
	ERROR: Invalid Number of Arguments.
	Total number of arugemts should be 2.
	The order of the arguments should be:
	  1) Path to the file.
	  2) String to be written in the specified file.
	EOF
	exit 1
fi

#tries to creates file directory
mkdir -p "$CDIR"
touch "$WRITEFILE"

#validates the file creation and writes the string to it
if [ -f $WRITEFILE ]
then
	echo "$WRITESTR" > "$WRITEFILE"
	exit 0
else
	echo "Couldn't able to create the file"
	exit 1
fi
