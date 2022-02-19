/* @file aesdsocket.c
 * @brief socket program
 *
 * Opens stream socket bound to 9000, Listens for and accepts connection
 * Receives data over the connection and appends to file /var/tmp/aesdsocketdata
 * Returns the full content of  /var/tmp/aesdsocketdata to the client as soon as the received data packet completes
 * Restarts accepting connections from new clients forever in a loop until SIGINT or SIGTERM is received
 * 
 * usage: 1) ./aesdsocket 
 * 	  2) ./aesdsocket -d (runs as daemon)
 *	
 * @author Santhosh, santhosh@colorado.edu
 * @references:
 * 		https://beej.us/guide/bgnet/html/
 * 		http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
 * 		https://stackoverflow.com/questions/44789295/correct-use-of-realloc
 *		https://gcc.gnu.org/onlinedocs/gcc/Variadic-Macros.html
 */


/******************************************************************************
 *
 *                                  HEADERS
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <syslog.h>

/******************************************************************************
 *
 *                               DEFINITIONS
 *
 ******************************************************************************/
#define PORT "9000"
#define BACKLOG (10)
#define WRITE_FILE "/var/tmp/aesdsocketdata"
#define LOG_ERROR(msg, ...) 	syslog(LOG_ERR, msg, ##__VA_ARGS__) 
#define LOG_MSG(msg, ...)	syslog(LOG_INFO, msg, ##__VA_ARGS__) 
#define LOG_WARN(msg, ...)  	syslog(LOG_WARNING, msg, ##__VA_ARGS__)  
/******************************************************************************
 *
 *                                  GLOBALS
 *
 ******************************************************************************/

static int servfd, clientfd;
char *rdBuff=NULL;

/******************************************************************************
 *
 * 				    FUNCTIONS
 *
 ******************************************************************************/ 
void closeAll()
{

	// free read-write buffer
	if(rdBuff != NULL){
		free(rdBuff);
		rdBuff = NULL;
	}
		
	// close file descriptors	
	shutdown(servfd, SHUT_RDWR);
	close(servfd);
	shutdown(clientfd, SHUT_RDWR);
	close(clientfd);
	
	// delete the data file
	remove(WRITE_FILE);
	
	//close the logging
	closelog();
}

void signal_handler(int signo)
{
	LOG_WARN("Caught signal, exiting\n");
    	closeAll();
	exit(0);
    
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void runAsDaemon()
{

	pid_t pid, sid;
	
	// signals to be ignored
	signal(SIGCHLD, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
	
	//Fork the parent process
        pid = fork();
	
	if(pid < 0){
            perror("fork():");
            LOG_ERROR("fork(): %s", strerror(errno));
            closeAll();
            exit(EXIT_FAILURE);
        }
        
        //if no error, exit the parent process
        if(pid > 0)
            exit(EXIT_SUCCESS);

        // Change the file mode mask
        umask(0);


        // Creating new SID for the child process
        sid = setsid();
        if(sid < 0){
            perror("setsid()():");
            LOG_ERROR("setsid(): %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Change the current working directory
        if((chdir("/")) < 0){
            perror("chdir():"); 
            LOG_ERROR("chdir(): %s", strerror(errno));	 	
            exit(EXIT_FAILURE);
	}
	
        // redirect standard IO to /dev/null
    	int extf = open("/dev/null", O_RDWR);
   	dup2(extf, STDIN_FILENO);
    	dup2(extf, STDOUT_FILENO);
    	dup2(extf, STDERR_FILENO);
    	
        // close std file descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        close(extf);
}



int main(int argc, char **argv)
{
	
	openlog(NULL,0,LOG_USER);
	
	// register signals
	if(signal(SIGINT, signal_handler) == SIG_ERR){
		LOG_ERROR("Error while registering SIGINT");
		closeAll();
		return -1;
	}


	if(signal(SIGTERM, signal_handler) == SIG_ERR){
		LOG_ERROR("Error while registering SIGTERM");
		closeAll();
		return -1;
	}
	
	bool flagDaemon = false;
	if(argc >=2 && (strcmp(argv[1],"-d") == 0))
		flagDaemon = true;
	else if(argc >=2){
		printf("Invalid arguments\n");
		LOG_ERROR("Invalid arguments");
		exit(1);	
	}
	
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;


	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;     
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags = AI_PASSIVE;     

	// get address info
	rv = getaddrinfo(NULL, PORT , &hints, &servinfo);
	
	// get address info error check
	if (rv  != 0) {
    		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
    		LOG_ERROR("getaddrinfo error: %s\n", gai_strerror(rv));
    		exit(1);
	}


	// loop through all the results and bind to the first we can
    	for(p = servinfo; p != NULL; p = p->ai_next) {
        	if ((servfd = socket(p->ai_family, p->ai_socktype,
                	p->ai_protocol)) == -1) {
            		perror("server: socket");
            		LOG_ERROR("server: socket: %s", strerror(errno));
            		continue;
        	}

        	if (setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                	sizeof(int)) < 0 ) {
            		perror("setsockopt");
            		LOG_ERROR("setsocket: %s", strerror(errno));
            		exit(1);
        	}

        	if (bind(servfd, p->ai_addr, p->ai_addrlen) == -1) {
            		close(servfd);
            		perror("server: bind");
            		LOG_ERROR("server: bind:  %s", strerror(errno));
            		continue;
        	}

        	break;
    	}

    	freeaddrinfo(servinfo); // all done with this structure 
	
	if (p == NULL)  {
        	fprintf(stderr, "server: failed to bind\n");
        	LOG_ERROR("server:failed to bind: %s", strerror(errno));
        	exit(1);
    	}


	if(flagDaemon)
		runAsDaemon();
    	
    	if (listen(servfd, BACKLOG) == -1) {
        	perror("listen");
        	LOG_ERROR("listen: %s", strerror(errno));
        	exit(1);
    	}


	
    	printf("server: waiting for connections...\n");
	

	ssize_t totalBytes = 0;
	while(1) {  // main accept() loop
	
		char buffer[1024];
		rdBuff = malloc(sizeof(char));
		
		// Accept connection
		sin_size = sizeof their_addr;
		clientfd = accept(servfd, (struct sockaddr *)&their_addr, &sin_size);
		if (clientfd == -1) {
		    perror("accept");
		    continue;
		}

		inet_ntop(their_addr.ss_family,
		    get_in_addr((struct sockaddr *)&their_addr),
		    s, sizeof s);
		printf("server: got connection from %s\n", s);
		LOG_MSG("Accepted connection from %s", s);


		int fd = open(WRITE_FILE, O_CREAT, 0644);	
		if(fd < 0){
			LOG_ERROR("Error opening %s",WRITE_FILE);
			closeAll();
			return -1;
		}
		close(fd);
		
		// Receive and write to WRITE_FILE
		fd =open(WRITE_FILE,O_WRONLY);
		if(fd < 0){
			LOG_ERROR("Error opening %s",WRITE_FILE);
			closeAll();
			return -1;
		}
		
	        lseek(fd,0,SEEK_END);
	        unsigned long index = 0;
		while(1){
		
			ssize_t rBytes = recv(clientfd, buffer, sizeof(buffer), 0);
			if(rBytes == -1){
				LOG_ERROR("recv():%s", strerror(errno));
				closeAll();
				return -1;
			}
			rdBuff = realloc(rdBuff,index+rBytes);
			memcpy(rdBuff+index,buffer,rBytes);
			index += rBytes;
			totalBytes += rBytes;
			if(buffer[rBytes-1] == '\n'){
				
				if(write(fd, rdBuff, index) == -1){
					LOG_ERROR("write():%s", strerror(errno));
					closeAll();
					return -1;
				}
				break;
			}
		}
		close(fd);	
		free(rdBuff);
		rdBuff=NULL;	
		
		// Send complete data from WRITE_FILE to the client
		fd =open(WRITE_FILE,O_RDONLY);
		if(fd < 0){
			LOG_ERROR("Error opening %s",WRITE_FILE);
			closeAll();
			return -1;
		}
		ssize_t sendBytes = totalBytes;
		rdBuff = realloc(rdBuff,sendBytes);
		lseek(fd,0,SEEK_SET);
		while(1){		
			
			int rBytes = read(fd, rdBuff,sendBytes);
			if(rBytes == -1){
				LOG_ERROR("read():%s", strerror(errno));
				closeAll();
				return -1;
			}
			if(send(clientfd, rdBuff, rBytes, 0) == -1){
				LOG_ERROR("send():%s", strerror(errno));
				closeAll();
				return -1;
			}
			sendBytes -= rBytes;
			if(sendBytes == 0)
				break;
		}
		free(rdBuff);
		rdBuff=NULL;
		close(fd);
		close(clientfd);
		LOG_MSG("Closed connection from %s", s);
		printf("server: closed connection with %s\n", s);
		

    	}
    	
	closeAll();
	return 0;
}
