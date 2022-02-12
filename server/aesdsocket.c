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

#define PORT "9000"
#define BACKLOG (10)
#define WRITE_FILE "/var/tmp/aesdsocketdata"

static int servfd, clientfd;


// TODO: Add -d support to run as daemon

void closeAll()
{
	close(servfd);
	close(clientfd);
}


void sigchld_handler(int signo)
{
    // waitpid() might overwrite errno, so we save and restore it:
    //int saved_errno = errno;
    closeAll();
    //while(waitpid(-1, NULL, WNOHANG) > 0);
	exit(1);
    //errno = saved_errno;
    
    // TODO: delete WRITE_FILE
    
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main()
{
	printf("Hello AESDSOCKET!\n");

	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
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
    		exit(1);
	}
	printf("getaddinfor() returnd %d: LOOKS NORMAL\n",rv);


	// loop through all the results and bind to the first we can
    	for(p = servinfo; p != NULL; p = p->ai_next) {
        	if ((servfd = socket(p->ai_family, p->ai_socktype,
                	p->ai_protocol)) == -1) {
            		perror("server: socket");
            		continue;
        	}

        	if (setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                	sizeof(int)) < 0 ) {
            		perror("setsockopt");
            		exit(1);
        	}

        	if (bind(servfd, p->ai_addr, p->ai_addrlen) == -1) {
            		close(servfd);
            		perror("server: bind");
            		continue;
        	}

        	break;
    	}

    	freeaddrinfo(servinfo); // all done with this structure 
	
	if (p == NULL)  {
        	fprintf(stderr, "server: failed to bind\n");
        	exit(1);
    	}

    	if (listen(servfd, BACKLOG) == -1) {
        	perror("listen");
        	exit(1);
    	}

    	sa.sa_handler = sigchld_handler; // reap all dead processes
    	sigemptyset(&sa.sa_mask);
    	sa.sa_flags = SA_RESTART;
    	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        	perror("sigaction");
        	exit(1);
    	}

    	printf("server: waiting for connections...\n");



	ssize_t total_b = 0;
	while(1) {  // main accept() loop
	
		char buffer[1024];
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


		int fd = open(WRITE_FILE, O_CREAT, 0644);	
		close(fd);
		
		// Receive and write to file
		fd =open(WRITE_FILE,O_WRONLY);
	        lseek(fd,0,SEEK_END);
		while(1){
		
			ssize_t rBytes = recv(clientfd, buffer, sizeof(buffer), 0);
			//printf("Received %ld bytes\n",rBytes);
			write(fd, buffer, rBytes);
			total_b += rBytes;
			if(buffer[rBytes-1] == '\n')
				break;
		}
		close(fd);		
		
		//printf("%ld\n",total_b);
		// Send 
		fd =open(WRITE_FILE,O_RDONLY);
		ssize_t rd = total_b;
		lseek(fd,0,SEEK_SET);
		while(1){		
			
			int r = read(fd, buffer,rd);
			//ssize_t sBytes = 
			send(clientfd, buffer, r, 0);
			//printf("Sent %ld bytes\n",sBytes);
			rd -= r;
			if(rd == 0)
				break;
		}
		close(fd);
		close(clientfd);
		printf("server: closed connection with %s\n", s);
		

    	}
    	
	closeAll();
	return 0;
}
