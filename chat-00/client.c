#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>
	
#include "gui.h"

#define BUFFERSIZE 1024


int main(int argc, char const *argv[]){
	int sockfd, port;
	struct pollfd fds[2];
	const char *hostname;
	struct sockaddr_in serverAddr;
    
  init_gui();   
	drawWindows();
	
	//check parameters
	if(argc!=3){
		exit_window("./client hostname port");
	}else{
		port = atoi(argv[2]);
		hostname = argv[1];	
		if(strcmp(hostname,"localhost")==0)
			hostname = "127.0.0.1";
	}
	
	//create socket and set info
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1)
		exit_window("Could not create socket");
	else
		printOutputW("","Socket created successfully\n");
	serverAddr.sin_addr.s_addr = inet_addr(hostname);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	
	//connect to server
	if(connect(sockfd,(struct sockaddr *)&serverAddr,sizeof(serverAddr))<0)
		exit_window("Connection not possible");
	else
		printOutputW("","Connected to Server\n");
	
	//set stdin and server in fds 
	fds[0].fd = sockfd;
  fds[0].events = POLLIN;
	fds[1].fd = STDIN_FILENO;
	fds[1].events = POLLIN;

	char buffer[BUFFERSIZE];
	printOutputW("","Ready to send and receive...\n");
	while(true){
		int state = poll(fds, 2, -1);
		if(state <= 0)
			continue;
		
		//listen from server and output
		if(fds[0].revents & POLLHUP)
			exit_window("connection to server lost");
		if(fds[0].revents & POLLERR)
			exit_window("error on server side");
		if(fds[0].revents & POLLIN){
			int n = read(fds[0].fd, buffer, BUFFERSIZE);
			if(n < 0)
				exit_window("error on server side");
			if(n == 0)
				exit_window("connection to server lost");	
			if(n > 0)
				printOutputW("", buffer);				
		}

		//read from stdin and write to server
		if(fds[1].revents & POLLIN){
			char *c = handle_input();
			if(strlen(c)>1 && (write(sockfd,c,strlen(c)+1))<0){
				exit_window("Error while sending");
			}
		}
		fflush(stdin); 
	}
	
	exit_window("");
	printf("Closed socket");	
	return 0;
}
