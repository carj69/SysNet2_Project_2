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
#define PORT 60010 
#define BUFFER_SZ 2048
#define LENGTH 2048

char cName[BUFFER_SZ];
int flag = 0;
int in_chat = 0;
int sockfd;
char name[32];

void str_trim_lf (char* arr, int length) {
    	int i;
    	for (i = 0; i < length; i++) { // trim \n
        	if (arr[i] == '\n') {
            	arr[i] = '\0';
            	break;
        	}
    	}
}

void str_overwrite_stdout() {
	printf("\r%s", "> ");
	fflush(stdout);
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
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

void send_msg_handler() {
  	char message[LENGTH] = "";
	char buffer[LENGTH + 32] = "";

  	while(1) {
	  		str_overwrite_stdout();
	    	fgets(message, LENGTH, stdin);
	    	str_trim_lf(message, LENGTH);
	
    		
			sprintf(buffer, "%s: %s\n",cName, message);
			send(sockfd, buffer, strlen(buffer), 0);
    		

			bzero(message, LENGTH);
    		bzero(buffer, LENGTH + 32);
  	}
  	catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
	char message[LENGTH] = "";
	printf("\nNow receving\n");
	while (in_chat) {
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
			bzero(message, LENGTH);
	}
	 printf("Stop receiving from server\n");
}

void goToPrivateChat() {
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
}


void handleGroupChat() {

	in_chat = 1;
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
}

void fileTransfer(){
		//char fileName[15];
		//printf("Enter file name you want to transfer:");
		//scanf("%s",fileName);
		//send(sockfd,fileName, MAX, 0);
	//send(sockfd, "File Transfer", strlen("File Transfer"), 0);	
	
	FILE *f;
    char buffer[2048];
    int words = 0;
    int n = 0;
    
    f=fopen("glad.txt","r");
    
    fscanf(f, "%[^\n]", buffer);
	printf("this is what being sent: %s\n",buffer);
	send(sockfd, buffer,strlen(buffer), 0);

	printf("The file was sent successfully");
	fclose(f);
	 //free(word);
}

void sendToMainMenu() {
	int option;
	char server_response[LENGTH] = "";
	
	while(1) {	
		memset(server_response, 0, strlen(server_response));
		printMainMenu();
		scanf("%d", &option);
		switch(option) {
			case 1:
				printf("\n-=| View Current Online Users |=-\n");
				send(sockfd, "View Online", MAX, 0);
				memset(server_response, 0, strlen(server_response));			
				recv(sockfd, &server_response, LENGTH, 0);
				printf("The number of current online users is: %s\n", server_response);
				break;
			case 2:
				printf("\n-=| Group Chat |=-\n");
				send(sockfd, "Enter Group", MAX, 0);
				handleGroupChat();
				break;
			case 3:	
				printf("\n-=| Private Chat |=-\n");
				send(sockfd, "Enter Private", MAX, 0);
				goToPrivateChat();
				break;
			case 4:
				printf("View chat history");
				send(sockfd, "View History", MAX, 0);
			
				break;
			case 5:
				printf("File transfering");
				send(sockfd, "File Transfer", MAX, 0);
				fileTransfer();
				break;
			case 6:
				printf("Change password");
				send(sockfd, "Change Password", MAX, 0);
			
				break;
			case 7:
				printf("Log out");
				send(sockfd, "Logout", MAX, 0);
				return;
			case 8:
				printf("admin");
				send(sockfd, "Administrator", MAX, 0);
				break;
			case 0:
				printf("Exiting main menu");
				send(sockfd, "Return", MAX, 0);
				break;
			default:
				printf("Try again\n");
				break;				
		}
	}	
}


int main() 
{ 
	// create the TCP socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	// define address and port of remote socket
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);
	
	// Connect to the Scoket
	if(connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
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
				send(sockfd, "register", strlen("register"), 0);	
				
				memset(buff,0,strlen(buff));
				printf("Create UserName: ");
				scanf("%s", buff);
				send(sockfd, buff, strlen(buff), 0);
				
				memset(buff,0,strlen(buff));
				printf("Create Password: ");
				scanf("%s", buff);
				send(sockfd, buff, strlen(buff), 0);

				// check/update database
				memset(server_response,0,strlen(server_response));
        			recv(sockfd, &server_response, sizeof(server_response), 0);
				printf("Server responds: %s\n", server_response);
				break;
			/* User Needs to Login */
			case 2:
				send(sockfd, "login", strlen("login"), 0);
                                
				memset(buff,0,strlen(buff));
				memset(cName,0,strlen(cName));
				printf("UserName: ");
                                scanf("%s", buff);
				strcpy(cName, buff);
                                send(sockfd, buff, strlen(buff), 0);

				memset(buff,0,strlen(buff));	
				printf("Password: ");
                                scanf("%s", buff);
                                send(sockfd, buff, strlen(buff), 0);

                                // check/update database
                                memset(server_response,0,strlen(server_response));
				recv(sockfd, &server_response, sizeof(server_response), 0);
                                printf("Server responds: %s\n", server_response);
			
				if(strcmp(server_response, "Log on success")==0)
					sendToMainMenu();
				
				break;
			/* User needs to disconnect */
			case 0:
				printf("Exiting...\n");
				send(sockfd, "exit", strlen("exit"), 0);
				close(sockfd);
				return EXIT_SUCCESS;
			default:
				break;
		}
	}
	// Close Server
	close(sockfd);
    	
    	return 0;
}
