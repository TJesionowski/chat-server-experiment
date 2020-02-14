all: server client

server: server.c
	gcc -Wextra -pedantic -lpthread -lrt server.c -o server
client: client.c
	gcc -Wextra -pedantic client.c -o client
