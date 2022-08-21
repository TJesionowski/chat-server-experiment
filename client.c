#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <netinet/ip.h>

#include <linux/sockios.h>

#include "server.h"

void check_errno(void) {
    if (errno != 0) {
	fprintf(stderr, "%s\n", strerror(errno));
    }
}

int main(void) {
    atexit(check_errno);

    // create the socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // connect to the server on localhost
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    // establish a connection
    printf("Connecting to server...\n");
    if (0 > connect(sockfd, (struct sockaddr*) &address, sizeof(address))) {
	fprintf(stderr, "ERROR: connect failed\n");
	exit(EXIT_FAILURE);
    }
    printf("Connection established.\n");

    client_id_t id;
    read(sockfd, &id, sizeof(id));

    const int bufsize = 1024;
    char buffer[bufsize];

    for (;;) {
	// send a message
	printf("%d: ", id);
	scanf("%s", &buffer);
	write(sockfd, buffer, bufsize);

	// recieve and print reply
	/* read(sockfd, buffer, bufsize); */
	/* printf("0: %s\n", buffer); */
    }

    return 0;
}
