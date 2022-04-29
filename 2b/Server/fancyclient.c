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

#include "fancyclient.h"

const int LINE_SIZE = 80;

int main (int argc, char* argv[]) {

    if (argc < 3) {
        if (argc == 1)
            fprintf(stderr, "Missing arguments. Please enter IP_ADDRESS and PORT_NUMBER.\n");
        else
            fprintf(stderr, "Missing argument. Please enter the PORT_NUMBER.\n");
        return 1;
    } // make sure all inputs are available

    // assign values
    char *dest_add = argv[1];
    char *dest_port = argv[2];

    // assign message strings
    char *msg = malloc(sizeof(char)*LINE_SIZE);
    printf("Line can take maximum 80 characters.\nInput message here:\n");
    fgets(msg, LINE_SIZE, stdin);

    // send UDP message
    int ret = send_UDP(dest_add, dest_port, msg);
    free(msg);
    return ret;
}


int send_UDP (char *dest_add, char *dest_port, char *msg) {

    int port = strtol(dest_port, NULL, 10); // referred from delftstack.com
    // referred from man page
    struct sockaddr_in IPv4;
    IPv4.sin_family = AF_INET;
    IPv4.sin_port = htons(port);
    // IPv4.sin_zero = NULL;

    // referred from Moodle
    int valid_add = inet_pton(AF_INET, dest_add, &IPv4.sin_addr);

    if (valid_add <= 0) {
        fprintf(stderr, "Invalid IPv4 address\n");
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Socket, %s - %d\n", strerror(errno), errno);
        return 1;
    }

    int sent = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&IPv4, sizeof(IPv4)); // idea from IBM
    if (sent < 0) {
        fprintf(stderr, "Error in sending data: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        return 1;
    }

    // receiving memory
    char *buf = malloc(LINE_SIZE+1);
    socklen_t server_len;
    server_len = sizeof(IPv4);

    // receive message
    int recvd = recvfrom(sockfd, buf, LINE_SIZE, 0, (struct sockaddr *)&IPv4, &server_len);
    if (recvd < 0) {
        fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        return 1;
    }

    fprintf(stdout, "Received message from server: %s", buf);

    free(buf);
    close(sockfd);
    return 0;
}