/*******************************************************
 *  S22-AESD-Assignment-2
 *  
 *  @file writer.c
 *  @description Application to wrtie a string to a specified file
 *  @usage ./write <file path> <string>
 *
 *  @author Santhosh, santhosh@colorado.edu
 *  @created on January 18, 2022
 ********************************************************/

#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_ARG (3)

int main( int argc, char *argv[] )
{

	// syslog using LOG_USER facility
	openlog( NULL, 0, LOG_USER);

	// check input number of arguments
	if( argc != MAX_ARG){
		printf("ERROR: Invalid Number of Arguments\n"   \
		       "Total number of arguments should be 2\n"\
		       "Usage: ./writer <file path> <string>\n"      
		       );
		syslog(LOG_ERR,"Invalid number of arguments: %d",argc-1);
		return 1;
	}	

	const char *writefile, *writestr;  
	writefile = argv[1];
	writestr = argv[2];

	// Create/open the file
	int fp;
	fp = open( writefile, O_CREAT |  O_WRONLY, 0644);

	// check file creation/opening failure
	if( fp == -1){
		syslog(LOG_ERR,"Couldn't able to create/open the file %s", writefile);
		return 1;
	}

	// Write string to the file
	ssize_t len;
	len = write( fp, writestr, strlen(writestr) );
	
	if(len == -1)
		// file writing failure
		syslog(LOG_ERR,"%s: File write failed",writefile);
	else if( len != strlen(writestr) )
		// Partial file write
		syslog(LOG_ERR,"%s: Partial file write", writefile);
	else
		syslog(LOG_DEBUG,"Writing %s to %s", writestr, writefile);
	
	// Close file
	if(close(fp) == -1)
		syslog(LOG_ERR,"Failed to close the file %s",writefile);
		
	// close syslog
	closelog();
	
	return 0;
}

