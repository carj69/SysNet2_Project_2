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
#define MAX_CLIENTS 100

/* Client structure */
typedef struct {
    struct sockaddr_in addr; /* Client remote address */
    int connfd;              /* Connection file descriptor */
    int uid;                 /* Client unique identifier */
    char name[32];           /* Client name */
} client_t;

client_t *clients[MAX_CLIENTS];

char FILEPATH[100] = "useracounts.csv";
int USER_FOUND = 1;
int USER_NOT_FOUND = 0;

FILE *fptr;
int currentOnline = 0;

void strip_newline(char *s){
    while (*s != '\0') {
        if (*s == '\r' || *s == '\n') {
            *s = '\0';
        }
        s++;
    }
}

int userNameExist(char* name){
	fptr = fopen(FILEPATH, "r");
        if(fptr == NULL){
                printf("File not found\n");
		return USER_NOT_FOUND;
	}
	char buff[2048] = "";	
	while(fgets(buff, 2048, fptr)) {
		int field_count = 0;
		char* field = strtok(buff, ",");
		while(field) {
			if(field_count == 0 && strcmp(name, field) == 0) {
				printf("User [%s] found\n", field);
				fclose(fptr);
				return USER_FOUND;
			}
			field = strtok(NULL, ",");
			field_count++;
		}
	}
	printf("User not Found\n");
	fclose(fptr);
	return USER_NOT_FOUND;
}

/* Add client to csv file */
int userRegister(char* username, char* password){
	// check file for username
	if(userNameExist(username))
		return USER_FOUND;
	fptr = fopen(FILEPATH, "a");
        if(fptr == NULL)
        	printf("File error");
 	fputs(username, fptr);
	fputs(",", fptr);
	fputs(password, fptr);
	fputs("\n", fptr);

	fclose(fptr);
	return USER_NOT_FOUND;
}
/* Check file */
int userLogin(char* name, char* password){
	fptr = fopen(FILEPATH, "r");
        if(fptr == NULL){
                printf("File not found\n");
                return USER_NOT_FOUND;
        }
        char buff[2048] = "";
        while(fgets(buff, 2048, fptr)) {
                int field_count = 0;
                char* field = strtok(buff, ",");
                while(field) {
                        if(field_count == 0 && strcmp(name, field) == 0) {
                                printf("User [%s] found\n", field);
                                field = strtok(NULL, ",");
				strip_newline(field);
				printf("Comparing Password [%s] with [%s]\n", password, field);
				if(strcmp(password, field) == 0){
					fclose(fptr);
					return USER_FOUND;
				} else {
					printf("Incorrect Password\n");
					fclose(fptr);
                                	return USER_NOT_FOUND;
				}
                        }
                        field = strtok(NULL, ",");
                        field_count++;
                }
        }
        printf("User not Found\n");
        fclose(fptr);
        return USER_NOT_FOUND;
}


/* Send message to a client from a client */

/* Send message to all clients but the sender */
void handleGroupChat(int connfd) {
	
}

/* Send message to sender */

/* Handles all communication with the client in Main Menu*/
void handleCommunication(int connfd) {
	char buff[2048] = "";
     	long valread = read(connfd, buff, 2048);
	strip_newline(buff);
	if(strcmp(buff, "View Online") == 0) {
		char result[50];
		sprintf(result, "%d", currentOnline);
		send(connfd, result, sizeof(int), 0);
	}else if(strcmp(buff, "Enter Group") == 0){
		handleGroupChat(connfd);
	}else if(strcmp(buff, "Enter Private") == 0){

	}else if(strcmp(buff, "View History") == 0){

	}else if(strcmp(buff, "File Transfer") == 0){

	}else if(strcmp(buff, "Change Password") == 0){	

	}else if(strcmp(buff, "Logout") == 0){

	}else if(strcmp(buff, "Administrator") == 0){

	}else if(strcmp(buff, "Return") == 0){

	}else {
		printf("[Main Menu]Unknown responce: %s\n",buff);
	}
}

/* Handles Server starting sequence */
int main() {
	int listenfd = 0, connfd = 0; 
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	
	// Server settings
	listenfd = socket(AF_INET,SOCK_STREAM,0);
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
	if(listen(listenfd, MAX_CLIENTS) == -1)
	{
		perror("Socket listing failed");
		return EXIT_FAILURE;
	}

	printf("-=| Server started |=-\n");
     
	// Accepts clients
	while(1) 
	{
		// Connecting client to server
		socklen_t client_len = sizeof(client_addr);
		connfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
		if(connfd == -1)
		{
			perror("connection fail....\n");
			continue;
		}
		printf("Client connection Successful\n");
		
		while(1) {	
			// Send Data
			char buff1[2048] = "";
			char buff2[2048] = "";
			long valread = read(connfd, buff1, 2048);
			if(strcmp(buff1, "login") == 0) {
				memset(buff1,0,strlen(buff1));
				printf("Client loggin on\n");
				read(connfd, buff1, 2048);
                                printf("Username: %s\n", buff1);
                                read(connfd, buff2, 2048);
                                printf("Password: %s\n", buff2);
				
				//check csv fil
				if(userLogin(buff1, buff2) == USER_FOUND){
					currentOnline++;
					send(connfd, "Log on success", 50,0);
					handleCommunication(connfd);
				}
                                else
                                        send(connfd, "Log on failed", 50,0);

			} else if(strcmp(buff1, "register") == 0) {
				memset(buff1,0,strlen(buff1));
				printf("Client registering\n");
				read(connfd, buff1, 2048);
				printf("Username: %s\n", buff1);
				read(connfd, buff2, 2048);
				printf("Password: %s\n", buff2);

				if(userRegister(buff1, buff2) == USER_FOUND)
					send(connfd, "Username already exits", 50,0);
				else
					send(connfd, "Register successful", 50,0);

			} else if (strcmp(buff1, "exit") == 0) {
			       	printf("Client exiting...\n");
		       		break;	       
			}else if(valread > 0){
				printf("[Login Menu] Unknown response: %s\n",buff1);
			}
		}
		/* Decrease server CPU usage */
		sleep(1);
	}
	return EXIT_SUCCESS;

	return 0;
}
