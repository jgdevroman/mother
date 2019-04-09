#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <fcntl.h>

#include <signal.h>

#include "request.h"
#include "connection.h"
#include "cmdline.h"
#include "jbuffer.h"


static BNDBUF *buf;


/* Helper function that set flags for close-on-exec 
 * If the given socket is not valid, the socket gets closed and -1 
 * will be returned. */
static int cle(int fd){
	int flags = fcntl(fd, F_GETFD,0);
	if(flags == -1){
		perror("fcntl getflag");
		if(errno == EINVAL)
			exit(EXIT_FAILURE);
		close(fd);
		return -1;
	}
	errno = 0;
	if(fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1){
		perror("fcntl setflag");
		if(errno == EINVAL)
			exit(EXIT_FAILURE);
		close(fd);
		return -1;
	}
	
	return fd;
}

static void *worker(void *arg){
	
	while(1){
		int sock = bbGet(buf);
		
		FILE *rx;
		FILE *tx;
		
		//reading stream
		if((rx = fdopen(sock, "r")) == NULL){
			perror("fdopen read");
			close(sock);
			continue;
		}
		
		//duplicate socket descriptor for writing stream
		int sockcpy = dup(sock);
		if(sockcpy == -1){
			perror("dup");
			fclose(rx);
			continue;
		}
		
		//close sockcpy on exec
		sockcpy = cle(sockcpy);
		if(sockcpy == -1){
			fclose(rx);
			continue;
		}
		
		
		//writing stream
		if ((tx = fdopen(sockcpy, "w")) == NULL){
			perror("fdopen write");
			close(sockcpy);
			fclose(rx);
			continue;
		}
		
		handleRequest(rx,tx);
		
		fclose(rx);
		fclose(tx);
		
	}
	return NULL;
}



int initConnectionHandler(){
	size_t t;
	size_t b;
	
	//initialize requesthandler
	if (initRequestHandler() == -1){
		return -1;
	}
	
	//ignore SIGPIPE
	struct sigaction sig;
	sig.sa_handler = SIG_IGN;
	sig.sa_flags = 0;
	sigemptyset(&sig.sa_mask);
	if(sigaction(13, &sig, NULL) == -1){
		perror("sigaction SIGPIPE");
		exit(EXIT_FAILURE);
	}
	
	//collect zombies
	struct sigaction sigc;
	sigc.sa_handler = SIG_DFL;
	sigc.sa_flags = SA_NOCLDWAIT;
	sigemptyset(&sigc.sa_mask);
	if(sigaction(SIGCHLD, &sigc, NULL) == -1){
		perror("sigaction SIGCHLD");
		exit(EXIT_FAILURE);
	}
	
	//get amount of threads and size of buffer from cmdline
	const char *threads = cmdlineGetValueForKey("threads");
	if(threads ==  NULL){
		t = 4;
	}else{
		errno = 0;
		long long get = strtoll(threads, NULL, 0);
		if(errno == ERANGE || get < 1 || get > SIZE_MAX || 
		*threads == '\0'){
			return -1;
		}
		if (errno != 0){
			perror("strtol threads");
			exit(EXIT_FAILURE);
		}
		t = (size_t) get;
	}
	
	const char *bufsize = cmdlineGetValueForKey("bufsize");
	if(bufsize == NULL){
		b = 8;
	}else{
		errno = 0;	
		long long get = strtoll(bufsize, NULL, 0);
		if (errno == ERANGE || get < 1 || get > SIZE_MAX || 
		*bufsize == '\0'){
			return -1;
		}
		if (errno != 0){
			perror("strtol bufsize");
			exit(EXIT_FAILURE);
		}
		b = (size_t) get;
	}
	
	//create jbuffer
	if ((buf = bbCreate(b)) == NULL){
		fprintf(stderr, "bbCreate failed!\n");
		exit(EXIT_FAILURE);
	}
	
	//create threads
	for (int i = 0; i < t; i++){
		pthread_t tid;
		errno = pthread_create(&tid, NULL, worker, NULL);
		
		if (errno != 0){
			perror("pthread_create");
			bbDestroy(buf);
			exit(EXIT_FAILURE);
		}
	}
	
	return 0;
}


void handleConnection(int clientSock, int listenSock){
	
	//Close clientSocket on exec
	clientSock = cle(clientSock);
	if(clientSock == -1){
		return;
	}
	
	//Close listenSock on exec
	listenSock = cle(listenSock);
	if(listenSock == -1){
		return;
	}
	
	//Put clientSocket into buffer	
	bbPut(buf, clientSock);
	return;
}
