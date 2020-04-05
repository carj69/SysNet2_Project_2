#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX 100
#define PORT 60001 
#define BUFFER_SZ 2048
#define LENGTH 2048

char cName[BUFFER_SZ];
int flag = 0;
int sockfd = 0;
char name[32];

void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_msg_handler() {
  	char message[LENGTH];
	char buffer[LENGTH + 32];

  	while(1) {
	  	str_overwrite_stdout();
	    	fgets(message, LENGTH, stdin);
	    	str_trim_lf(message, LENGTH);
	
    		if (strcmp(message, "exit") == 0) {
			break;
    		} else {
			sprintf(buffer, "%s: %s\n", name, message);
			send(sockfd, buffer, strlen(buffer), 0);
    		}

		bzero(message, LENGTH);
    		bzero(buffer, LENGTH + 32);
  	}
  	catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
	char message[LENGTH] = {};
	printf("\nNow receving\n");
	while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
		if (receive > 0) {
			printf("%s", message);
			str_overwrite_stdout();
		} else if (receive == 0) {
			printf("Stop receiving from server\n");
			break;
		} else {
			// -1
		}
		memset(message, 0, sizeof(message));
	}
}

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

void handleGroupChat(int client_fd) {
	// Need to send continously
	// Need to receive continously
	char buffer[BUFFER_SZ] = "";
	char message[BUFFER_SZ] = "";

	while(1) {
		str_overwrite_stdout();
		fgets(buffer, BUFFER_SZ, stdin);
		str_trim_lf(buffer, BUFFER_SZ);

		if (strcmp(buffer, "exit") == 0) {
			break;
		} else {
			sprintf(message, "%s: %s", cName, buffer);
			send(client_fd, message, strlen(message), 0);
		}
	}
}

void sendOptionToServer(int client_fd, int option) {
	
	char server_response[2048] = "";
	switch(option){
		case 1:
			printf("\n-=| View Current Online Users |=-\n");				
			send(client_fd, "View Online", MAX, 0);
			memset(server_response, 0, strlen(server_response));
			recv(client_fd, &server_response, sizeof(server_response), 0);
			printf("The number of current online users is: %s\n", server_response);
			break;
		case 2:
			printf("\n-=| Group Chat |=-\n");
			send(client_fd, "Enter Group", MAX, 0);	
			/* Sends messages to server */
			pthread_t send_msg_thread;
			if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
				printf("ERROR: pthread\n");
			}
			/* recieves messages to server */
			pthread_t recv_msg_thread;
			if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
				printf("ERROR: pthread\n");
			}
			
			pthread_join(send_msg_thread, NULL);
			pthread_join(recv_msg_thread, NULL);
			
			//handleGroupChat(client_fd);
			break;
		case 3:
			printf("\n-=| Private Chat |=-\n");
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
			printf("Try again\n");
			break;				
	}	
}


int main() 
{ 
	// create the TCP socket
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	sockfd = client_fd;
	
	// define address and port of remote socket
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	
	// Connect to the Scoket
	if(connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
		printf("Problem Connecting\n");
	}
	
	int input, menuOption;
	char buff[1024];
	char server_response[1024];
	while(1) {
		printMenu();
		scanf("%d",&input);
		
		switch(input){
			/* User needs to register with the server */
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
			/* User Needs to Login */
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
				
				while(strcmp(server_response, "Log on success") ==0){
					strcpy(cName, buff);
					printMainMenu();
					scanf("%d",&menuOption);
					sendOptionToServer(client_fd, menuOption);
					if(menuOption == 0)
						break;
				}
				
				break;
			/* User needs to disconnect */
			case 0:
				printf("Exiting...\n");
				send(client_fd, "exit", strlen("exit"), 0);
				close(client_fd);
				return EXIT_SUCCESS;
			default:
				break;
		}
	}
	// Close Server
	close(client_fd);
    	
    	return 0;
}
