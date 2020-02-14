#include <unistd.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <linux/sockios.h>

#include "server.h"

void init(void);

bool connection_waiting(void);
bool has_data(struct pollfd *client);

void add_client(void);
void remove_client(int client_index);

void check_error(int retcode, const char *message);
void check_errno(void);

int connections;
struct pollfd *server = NULL;
struct pollfd *clients = NULL;

int main(void) {
    atexit(check_errno);

    init();

    // establish a connection (only dealing with one client for now)
    /* int client; */
    /* int addrlen = sizeof(address); */
    /* if (0 > (client = accept(server, (struct sockaddr*) &address, &addrlen))) { */
    /* 	fprintf(stderr, "ERROR: acccept failed\n"); */
    /* 	exit(EXIT_FAILURE); */
    /* } */
    /* printf("Connection established.\n"); */

    for (;;) {
	check_error(poll(server, connections + 1, -1),
	    "poll failed");
	/* if (0 > read(client, buffer, bufsize)) { */
	/*     fprintf(stderr, "ERROR: read failed\n"); */
	/*     exit(EXIT_FAILURE); */
	/* } */
	/* printf("1: %s\n", buffer); */

	/* // reply is in here till I figure out threading */
	/* printf("0: "); */
	/* scanf("%s", &buffer); */
	/* if (0 > write(client, buffer, bufsize)) { */
	/*     fprintf(stderr, "ERROR: write failed\n"); */
	/*     exit(EXIT_FAILURE); */
	/* } */

	if (connection_waiting()) {
	    printf("Detected connection.\n");
	    add_client();
	    continue;
	}

	for (int i = 0; i < connections; i++) {
	    if (clients[i].revents & POLLIN) {
		int toread;
		check_error(ioctl(clients[i].fd, SIOCINQ, &toread), "ioctl failed");

		if (toread == 0) {
		    remove_client(i);
		} else {
		    char buf[BUFSIZE] = {};
		    read(clients[i].fd, buf, BUFSIZE - 1);

		    if (buf[0] != 0) // idfk why this is even a thing...
			printf("%d: %s\n", clients[i].fd, buf);
		}
	    }
	}
    }

    return 0;
}

void init(void) {
    // create the socket
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);

    // configure the socket
    int opt = 0;
    check_error(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)),
		"setsockopt failed");

    // bind the socket to a port on localhost
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    check_error(bind(serverfd, (struct sockaddr*) &address, sizeof(address)),
		"bind failed");

    // listen for connections
    printf("Listening for a connection...\n");
    check_error(listen(serverfd, 1),
		"listen failed");

    // Allocate memory for server pollfd
    server = malloc(sizeof(struct pollfd));
    memset(server, '\0', sizeof(struct pollfd));
    server->fd = serverfd;
    server->events = POLLIN;
}

bool connection_waiting(void) {
    return server->revents & POLLIN;
}

void add_client(void) {
    struct pollfd client = {0};
    client.events = POLLIN;

    check_error(client.fd = accept(server->fd, NULL, NULL),
		"accept failed");

    server = realloc(server, sizeof(struct pollfd) * (1 + ++connections));
    clients = &(server[1]);
    clients[connections - 1] = client;

    client_id_t id = client.fd;
    write(client.fd, &id, sizeof(id));
    printf("Added client on fd %d\n", client.fd);
}

void remove_client(int client_index) {
    printf("Removing connection with client on fd %d\n", clients[client_index].fd);

    // first, close the socket!
    close(clients[client_index].fd); // no error checking

    // now, iterate through the rest of the clients array, backfilling
    for (int i = client_index + 1; i < connections; i++) {
	clients[i - 1] = clients[i];
    }

    // shrink the clients array
    server = realloc(server, sizeof(struct pollfd) * connections);
    clients = &(server[1]);

    // finally, decrement the connection count
    connections--;
}

void check_error(int retcode, const char *message) {
    if (0 > retcode) {
	fprintf(stderr, "ERROR: %s\n", message);
	exit(EXIT_FAILURE);
    }
}

void check_errno(void) {
    if (errno != 0) {
	fprintf(stderr, "%s\n", strerror(errno));
    }
}
