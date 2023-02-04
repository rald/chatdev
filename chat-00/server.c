#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <poll.h>

#define BUFFERSIZE 1024
#define MAXCLIENTS 1000

int main (int argc, char *argv[]) {
  int server_fd, client_fd;
  struct sockaddr_in server, client;
	const int max_clients = MAXCLIENTS;
  struct pollfd clients[max_clients];
	char buffer[BUFFERSIZE];
	
  //check parameters		
  if (argc != 2){
		perror("./server port");
		exit(1);
	}

	//create socket and set info
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0){
		perror("Could not create socket");
		exit(1);
	}
  server.sin_family = AF_INET;
  server.sin_port = htons(atoi(argv[1]));
  server.sin_addr.s_addr = htonl(INADDR_ANY);
	int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);
  
	//assign adress to socket
  if(bind(server_fd, (struct sockaddr *) &server, sizeof(server))<0){
  	perror("Could not bind socket");
		exit(1);
	}
		
  if(listen(server_fd, 128)<0){
  	perror("Could not listen on socket\n");
		exit(0);
	}
  printf("Server is listening on %s\n", argv[1]);
	printf("exit with control-c\n");	
	
	//set other fd available
	for(int i = 1;i < max_clients; i++){
		clients[i].fd = -1;
	}
 		
	clients[0].fd = server_fd;
	clients[0].events = POLLIN;
	
	int client_count = 0;
  while (true) {
		int state = poll(clients, client_count+1, -1);
		if(state <= 0){
			continue;
		}
		
		//event on server socket
		if(clients[0].revents & POLLIN){	
			//accept client request and create new connection
  		socklen_t client_len = sizeof(client);
  		client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);
  		if (client_fd < 0){
				printf("Could not establish new connection\n");
  		}
			
			//assign client to fds array 
			for(int i = 0; i < max_clients; i++){
				if(clients[i].fd == -1){
					printf("Client %i assigned\n",i);
					clients[i].fd = client_fd;
					clients[i].events = POLLIN;
					client_count++;
					break;	
				}
				if(i == max_clients -1){
					printf("No more clients fd available\n");	
				}
			}	
		}	 			
		
    for(int i =1; i < client_count+1; i++){
			if(clients[i].fd == -1)
				continue;

			//client closed connection event
			if(clients[i].revents & POLLHUP){
				clients[i].revents = -1;
				clients[i].fd = -1;
				client_count--;	
				printf("Connection to Client %i ended\n", i);
			}
			
			//client input event
			if(clients[i].revents & POLLIN){
				memset(buffer,0,sizeof(buffer));
				
				//read input
				int r = read(clients[i].fd, buffer, sizeof(buffer));	
				if (r==0){
					printf("Connection to Client finished\n");
					clients[i].revents = -1;
					clients[i].fd = -1;
					client_count--;	
					break;
				}else if (r < 0){
					perror("Client read failed");
					exit(1);
				}
				printf("Client %i send: %s",i,buffer);
				
				//send message to everyone else
				for(int j = 1; j < client_count+1; j++){ 
					char msg[BUFFERSIZE];
					sprintf(msg, "Client %i: %s",i,buffer);		
					if(i != j && write(clients[j].fd, msg,BUFFERSIZE)<0){
						printf("Writing to Client %i failed\n",i);
					}
				}
		  }
		}
		//reset events
		for(int i = 0; i < MAXCLIENTS; i++){
			clients[i].revents = 0;
		}
	}
  return 0;
}
