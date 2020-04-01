#variable declaration :-
cc=gcc
MAKE=make
RM =rm

#targets .
all: chatClient.c chatServer.c
	$(cc) -o client chatClient.c	
	$(cc) -o server chatServer.c
	gnome-terminal -t server --working-directory=/home/techbeamers -e "./server"
	sleep 10s 
	$(MAKE) client_target	


#another target for client
client_target:
	./client  


clean:server client
	$(RM) server
	$(RM) client

