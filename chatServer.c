#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>        

#define PORT_NUMBER 60001
#define DATA_SIZE 2048
#define MAX_CLIENTS 10

/* Client Structure */

/* Add client to queue */

/* Delete client from queue */

/* Send message to a client from a client */

/* Send message to all clients but the sender */

/* Send message to sender */

/* Send list of active clients */

/* Handles all communication with the client */

/* Handles Server starting sequence */
int main() {
	int listenfd = 0; connfd = 0; 
	struct sockaddr_in server_addr;
	struct sockaddr_in clinet_addr;
	
	// Server settings
	int listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd < 0)
	{
		perror("fail");
		exit(1);
	}
	//socklen_t sin_len = sizeof(client_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT_NUMBER);

	// Ignore pipe siginals here?

	// Binding the socket to the IP address and port
	if(bind(listenfd,(struct sockaddr*) &server_addr,sizeof(server_addr)) < 0)
	{
		perror("Socket binding failed");
		return EXIT_FAILURE;
	}

	// Listening for connections
	if(listen(server_fd, MAX_CONNECTIONS) == -1)
	{
		perror("Socket listing failed");
		return EXIT_FAILURE
	}

	printf("-=| Server started |=-\n");
     
	// Accepts clients
	while(1) 
	{
		// Connecting client to server
		socklen_t client_len = sizeof(client_addr)
		connfd = accept(listenfd, (struct sockaddr*)&client_addr, &sin_len);
		if(client_fd == -1)
		{
			perror("connection fail....\n");
			continue;
		}
		printf("Client connection Successful\n");
			
		// Send Data
		char buff[2047] = "";
		long valread = read(client_fd, buff, 2047);

		if(valread > 0) {
			char data[2047] = "Hello from server";
			printf(buff);		
			send(client_fd, data, sizeof(data), 0); // Then send formatted response back to client
		}
	}
	close(server_fd);

	return 0;
}
