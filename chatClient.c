
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#define MAX 80 
#define PORT 60001 

int main() 
{ 
	// create the TCP socket
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	// define address and port of remote socket
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	
	// Connect to the Scoket
	if(connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
		printf("Problem Connecting\n");
	}
	
	// Send request to server
	char * test = "Hello from client";
    	send(client_fd, test, strlen(test) , 0 );

	// read server response
    	char server_response[1024];
	recv(client_fd, &server_response, sizeof(server_response), 0); 

	// Print out response
	printf("\n\n Server responds: %s\n", server_response);
	// Close Server
	close(client_fd);
    	
    	return 0;
}
