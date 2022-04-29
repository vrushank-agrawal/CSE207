#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "server.h"

const int LINE_SIZE = 80;

int main (int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Missing argument. Please enter the PORT_NUMBER.\n");
        return 1;
    } // make sure all inputs are available

    // assign values
    char *port = argv[1];

    // receive UDP message
    int ret = recv_UDP(port);
    return ret;
}


int recv_UDP (char *dest_port) {

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Socket, %s - %d\n", strerror(errno), errno);
        return 1;
    }

    int port = strtol(dest_port, NULL, 10); // referred from delftstack.com

    // inspiration from geeksforgeeks
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    int binded = bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    if (binded < 0){
        fprintf(stderr, "Error in binding connection: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        return 1;
    }

    // receiving memory
    char *buf = malloc(sizeof(char)*(LINE_SIZE+1));
    struct sockaddr client;
    socklen_t client_len;
    client_len = sizeof(client);

    while (1) {

        int recvd = recvfrom(sockfd, buf, LINE_SIZE, 0, &client, &client_len);
        if (recvd < 0) {
            fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            return 1;
        }

        int sent = sendto(sockfd, buf, recvd, 0, &client, client_len);
        if (sent < 0) {
            fprintf(stderr, "Error in sending confirmation message: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            return 1;
        }
    }

    free(buf);
    close(sockfd);
    return 0;
}