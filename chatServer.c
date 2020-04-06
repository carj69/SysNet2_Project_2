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
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>        

#define PORT_NUMBER 60010
#define DATA_SIZE 2048
#define MAX_CLIENTS 100
#define BUFFER_SZ 100

void strip_newline(char* name);

static _Atomic unsigned int currentOnline = 0;
static int uid = 10;

/* Client structure */
typedef struct {
    struct sockaddr_in address; /* Client remote address */
    int sockfd;              /* Connection file descriptor */
    int uid;                 /* Client unique identifier */
    char name[32];           /* Client name */
    int in_group;
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

char FILEPATH[100] = "useracounts.csv";
int USER_FOUND = 1;
int USER_NOT_FOUND = 0;

FILE *fptr;


void strip_newline(char *s){
    while (*s != '\0') {
        if (*s == '\r' || *s == '\n') {
            *s = '\0';
        }
        s++;
    }
}

void str_overwrite_stdout() {
    	printf("\r%s", "> ");
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

/* Add clients to online queue */
void queue_add(client_t *cl){
	pthread_mutex_lock(&clients_mutex);
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			currentOnline++;
			//printf("client sockfd: %d",clients[i]->sockfd);	
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Remove clients to online queue */
void queue_remove(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				currentOnline--;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


void leave_groupchat(int id) {
	pthread_mutex_lock(&clients_mutex);

        for(int i=0; i < MAX_CLIENTS; ++i){
                if(clients[i]){
                        if(clients[i]->uid == id){
                                clients[i]->in_group = 0;
                       		if (write(clients[i]->sockfd, "exit", strlen("exit")) < 0) {
                                        perror("Write to descriptor failed");
                                        break; 
  				}
				break;
                        }
                }
        }

        pthread_mutex_unlock(&clients_mutex);
}

void join_groupchat(int id) {
	pthread_mutex_lock(&clients_mutex);

        for(int i=0; i < MAX_CLIENTS; ++i){
                if(clients[i]){
                        if(clients[i]->uid == id){
                                clients[i]->in_group = 1;
                                break;
                        }
                }
        }

        pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients except sender in groupchat*/
void send_message_to_group(char *s, int uid){
	
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->in_group == 1 && clients[i]->uid != uid){
				strcat(s ,clients[i]->name);
				//printf("test sending message \n");
				send(clients[i]->sockfd, s, strlen(s), 0);
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Send message to a client */
void send_message(char *s, char* id){

        pthread_mutex_lock(&clients_mutex);

        for(int i=0; i<MAX_CLIENTS; ++i){
                if(clients[i]){
                        if(strcmp(clients[i]->name, id)){
                                strcat(s ,clients[i]->name);
                                //printf("test sending message \n");
                                send(clients[i]->sockfd, s, strlen(s), 0);
                        }
                }
        }

        pthread_mutex_unlock(&clients_mutex);
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
                                field = strtok(NULL, ",");
				strip_newline(field);
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

/* Send to a client */
void goToPrivateChat(client_t *cli) {
 	char to[BUFFER_SZ];
	char message[BUFFER_SZ];

	int receiver_name = recv(cli->sockfd, to, BUFFER_SZ, 0);	
	int receive_message; 
	while(1){
		if(userNameExist(to))
			printf("User found\n");

		receive_message = recv(cli->sockfd, message, BUFFER_SZ, 0);
 		if (receive_message > 0){
                        if(strlen(message) > 0){
				if(strcmp(message, "exit"))
					break;
                                send_message(message, to);
                                str_trim_lf(message, strlen(message));
                                printf("Private message from %s to %s\n", cli->name, to);
			}

                }

		bzero(message, BUFFER_SZ);	
	}
	
}


/* Send message to all clients but the sender */
void goToGroupChat(client_t *cli) {
	char buff_out[BUFFER_SZ];
	char name[50];
	int leave_flag = 0;
	
	join_groupchat(cli->uid);
	printf("Client:%s Id: %d joins the group chat\n", cli->name, cli->uid);	
	while(1){	
		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
		/* If message received */
		if (receive > 0){
			if(strlen(buff_out) > 0) {
				if(strcmp(buff_out, "exit"))
					break;
				send_message_to_group(buff_out, cli->uid);
				str_trim_lf(buff_out, strlen(buff_out));
				printf("Group message from %s: %s\n", cli->name, buff_out);
			}
		}else if (receive == 0){
			sprintf(buff_out, "%s has left\n", cli->name);
			printf("%s", buff_out);
			send_message_to_group(buff_out, cli->uid);
			leave_flag = 1;
		}		
		bzero(buff_out, BUFFER_SZ);
	}
	leave_groupchat(cli->uid);
}

/* Send message to a client from a client */

/* Send message to all clients but the sender */
void goToMenu(client_t *client) {
	printf("Client: [%s] Id: %d is in the Main Menu\n", client->name, client->uid);
    	char buff[2048] = "";
	while(1) {
		memset(buff, 0, strlen(buff));
		long valread = read(client->sockfd, buff, 2048);
		strip_newline(buff);
		if(strcmp(buff, "View Online") == 0) {
			printf("Client [%s] sends [%s] command\n", client->name, buff);
			char result[50];
			sprintf(result, "%d", currentOnline);
			send(client->sockfd, result, sizeof(int), 0);
		}else if(strcmp(buff, "Enter Group") == 0){
			printf("Client [%s] sends [%s] command\n", client->name, buff);
			goToGroupChat(client);
			printf("%s leaves group chat\n", client->name);
		}else if(strcmp(buff, "Enter Private") == 0){
			printf("Client [%s] sends [%s] command\n", client->name, buff);
			goToPrivateChat(client);	
		}else if(strcmp(buff, "View History") == 0){
			printf("Client [%s] sends [%s] command\n", client->name, buff);
				
		}else if(strcmp(buff, "File Transfer") == 0){
			printf("Client [%s] sends [%s] command\n", client->name, buff);
				
		}else if(strcmp(buff, "Change Password") == 0){	
			printf("Client [%s] sends [%s] command\n", client->name, buff);

		}else if(strcmp(buff, "Logout") == 0){
			printf("Client [%s] sends [%s] command\n", client->name, buff);
			
		}else if(strcmp(buff, "Administrator") == 0){
			printf("Client [%s] sends [%s] command\n", client->name, buff);
			
		}else if(strcmp(buff, "Return") == 0){
			printf("Client [%s] sends [%s] command\n", client->name, buff);

			break;	
		}else if(valread > 0){
			printf("[Main Menu]Unknown responce: %s\n",buff);
		}
	}
	currentOnline--;
}

/* Send message to sender */

/* Handles Client login and registeration */
void *handle_client(void *arg){

	client_t *cli = (client_t *)arg;
	int connfd = cli->sockfd;

	while(1) {
       		// Send Data
        	char buff1[2048] = "";
           	char buff2[2048] = "";
            	long valread = read(connfd, buff1, 2048);
         	
		/***************************** Client Logs in ************************************/
            	if(strcmp(buff1, "login") == 0) {
                	memset(buff1,0,strlen(buff1));
			printf("Client loggin on\n");
			read(connfd, buff1, 2048);
			printf("Username: %s\n", buff1);
			read(connfd, buff2, 2048);
			printf("Password: %s\n", buff2);

                	/* Check is user name/pwd is valid */
                	if(userLogin(buff1, buff2) == USER_FOUND){
				strcpy(cli->name, buff1);
				send(connfd, "Log on success", 50,0);
				
				/* Add client to the 0nline queue */
                		queue_add(cli);

				printf("Client:[%s] Id: %d has logged on\n", cli->name, cli->uid);
				goToMenu(cli);
			} else {
    				send(connfd, "Log on failed", 50,0);
			}
		/* Client regesters */
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
		/* Client exits */
		} else if (strcmp(buff1, "exit") == 0) {
             		printf("Client exiting...\n");
                   	break;
            	}else if(valread > 0){
            		printf("[Login Menu] Unknown response: %s\n",buff1);
      		}
	}
}


/* Handles Server starting sequence */
int main() {
	int listenfd = 0, connfd = 0; 
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	pthread_t tid;
	
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
		
		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = client_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;
		cli->in_group = 0;

		pthread_create(&tid, NULL, &handle_client, (void*)cli);

		/* Decrease server CPU usage */
		sleep(1);
	}
	return EXIT_SUCCESS;

	return 0;
}
