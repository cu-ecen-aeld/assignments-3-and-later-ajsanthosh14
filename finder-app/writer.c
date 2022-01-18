/*  S22-AESD-Assignment-2
 *  
 *  writer.c
 *
 *  Author: Santhosh, santhosh@colorado.edu
 */

#include <stdio.h>
#include <syslog.h>

int main( int argc, char *argv[] )
{

	if( argc != 3){
		printf("ERROR: Invalid Number of Arguments\n"   \
		       "Total number of arguments should be 2\n"\
		       "Usage: ./writer <file path> <string>\n"      
		       );

		return 1;
	}	



	//printf("%s %s\n", argv[2],argv[3]);

	//Creating a file
	FILE *fp;
	fp = fopen( argv[1] , "w" );
	fputs( argv[2] , fp );

	//Write string to the file
	fclose(fp);

	//Close file
	return 0;
}


