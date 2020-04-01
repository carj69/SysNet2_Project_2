
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#define MAX 80 
#define PORT 60001 


void printMenu() {
	printf("\n-=|  Online Chat Room  |=-\n");
	printf("1. Register \n");
	printf("2. Login\n");
	printf("0. QUIT \n\n");
	printf("Enter an action: ");
}

void printMainMenu() {

	printf("\n-=|  MAIN MENU  |=-\n");
	printf("1. View current online number\n");
	printf("2. Enter the group chat\n");
	printf("3. Enter the private chat\n");
	printf("4. View chat history\n");
	printf("5. File transfer\n");
	printf("6. Change the password\n");
	printf("7. Logout\n");
	printf("8. Administrator\n");
	printf("0. Return to the login screen\n\n");
	printf("Enter an action: ");
}


int currentOnline() {
	int numberOnline = 1;
		
	return numberOnline;
}

void selectMainMenu(int input) {
	
	switch(input){
			case 1:
				printf("Current of number of user onlin is: %d",currentOnline());				
				break;
			case 2:
				printf("entering group chat");
				break;
			case 3:
				printf("entering private chat");
				break;
			case 4:
				printf("View chat history");
				break;
			case 5:
				printf("File transfering");
				break;
			case 6:
				printf("Change password");
				break;
			case 7:
				printf("Log out");
				break;
			case 8:
				printf("admin");
				break;
			case 0:
				printf("Exiting main menu");
				break;
			default:
				break;
				
		}	
}

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
	
	int input, inputMainMenu;
	char buff[1024];
	char server_response[1024];
	while(1) {
		printMenu();
		scanf("%d",&input);
		
		switch(input){
			case 1:
				send(client_fd, "register", strlen("register"), 0);	
				
				memset(buff,0,strlen(buff));
				printf("Create UserName: ");
				scanf("%s", buff);
				send(client_fd, buff, strlen(buff), 0);
				
				memset(buff,0,strlen(buff));
				printf("Create Password: ");
				scanf("%s", buff);
				send(client_fd, buff, strlen(buff), 0);

				// check/update database
				memset(server_response,0,strlen(server_response));
        			recv(client_fd, &server_response, sizeof(server_response), 0);
				printf("Server responds: %s\n", server_response);
				break;
			case 2:
				send(client_fd, "login", strlen("login"), 0);
                                
				memset(buff,0,strlen(buff));
				printf("UserName: ");
                                scanf("%s", buff);
                                send(client_fd, buff, strlen(buff), 0);

				memset(buff,0,strlen(buff));	
				printf("Password: ");
                                scanf("%s", buff);
                                send(client_fd, buff, strlen(buff), 0);

                                // check/update database
                                memset(server_response,0,strlen(server_response));
				recv(client_fd, &server_response, sizeof(server_response), 0);
                                printf("Server responds: %s\n", server_response);
				
				while(strcmp(server_response, "Log on success")==0){
					printMainMenu();
					scanf("%d",&inputMainMenu);
					selectMainMenu(inputMainMenu);
					if(inputMainMenu == 0)
						break;
				}
				
				break;
			case 0:
				printf("Exiting...\n");
				send(client_fd, "exit", strlen("exit"), 0);
				close(client_fd);
				return EXIT_SUCCESS;
		}	

	}

	// Send request to server
	//char * test = "Hello from client";
    	//send(client_fd, test, strlen(test) , 0 );

	// read server response
    	//char server_response[1024];
	//recv(client_fd, &server_response, sizeof(server_response), 0); 

	// Print out response
	//printf("\n\n Server responds: %s\n", server_response);
	// Close Server
	close(client_fd);
    	
    	return 0;
}
