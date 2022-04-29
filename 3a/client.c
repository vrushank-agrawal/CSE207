#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "client.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main (int argc, char* argv[]) {

    if (argc < 3) {
        if (argc == 1)
            fprintf(stderr, "Missing arguments. Please enter IP_ADDRESS and PORT_NUMBER.\n");
        else
            fprintf(stderr, "Missing argument. Please enter the PORT_NUMBER.\n");
        return 1;
    } // make sure all inputs are available

    char *dest_add = argv[1];
    char *dest_port = argv[2];

    return send_UDP(dest_add, dest_port);
}

int send_UDP (char *dest_add, char *dest_port) {

    int port = strtol(dest_port, NULL, 10);
    sockaddr_in IPv4;
    IPv4.sin_family = AF_INET;
    IPv4.sin_port = htons(port);

    if (inet_pton(AF_INET, dest_add, &IPv4.sin_addr) <= 0) { // convert server address to network readable
        fprintf(stderr, "Invalid IPv4 address\n");
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Socket, %s - %d\n", strerror(errno), errno);
        return 1;
    }

    server_info *server = malloc(1 * sizeof(*server));
    server->IPv4 = (sockaddr_in *)malloc(sizeof(sockaddr_in *));
    server->IPv4 = &IPv4;
    server->sockfd = (int *)malloc(1 * sizeof(int));
    server->sockfd = &sockfd;
    char *msg = malloc(LINE_SIZE * sizeof(char));     // assign message strings
    pthread_t receive, send;

    printf("Input message here:\n");
    fgets(msg, LINE_SIZE, stdin);
    server->msg = msg; // assign message to server

    // printf("server msg : %s\n", server->msg);

    // send initial message
    if (pthread_create(&send, NULL, sendThread, server) != 0) {
        fprintf(stderr, "Error in creating sending thread: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        return 1;
    }

    // start receiving messages from the server
    if (pthread_create(&receive, NULL, rcvThread, server) != 0) {
        fprintf(stderr, "Error in creating receiving thread: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        return 1;
    }

    // join the send thread once message sent
    if (pthread_join(send, NULL) != 0) {
        fprintf(stderr, "Error in joining sending thread: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        return 1;
    }

    while (1) {
        printf("Input message here:\n");
        memset(msg, 0, sizeof(*msg));
        fgets(msg, LINE_SIZE, stdin);

        // send new message to server
        if (pthread_create(&send, NULL, sendThread, server) != 0 ) {
            fprintf(stderr, "Error in creating sending thread: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            return 1;
        }

        //close corresponding thread
        if (pthread_join(send, NULL) != 0) {
            fprintf(stderr, "Error in joining sending thread: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            return 1;
        }
    }

    // close the message receiving thread from server
    if (pthread_join(receive, NULL) != 0) {
        fprintf(stderr, "Error in joining receive thread: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        return 1;
    }

    free(msg);
    free(server->IPv4);
    free(server->sockfd);
    free(server);
    close(sockfd);
    return 0;
}

void *sendThread (void *arg) {
    server_info *server = (server_info *) arg;
    int *sockfd = server->sockfd;
    // printf("server msg in thread: %s\n", server->msg);
    if (sendto(*sockfd, server->msg, strlen(server->msg), 0, (struct sockaddr *)(server->IPv4), sizeof(*(server->IPv4))) < 0) {
        fprintf(stderr, "Error in sending data: %s :%d\n", strerror(errno), errno);
        pthread_exit(NULL);
    }
    pthread_exit(NULL);
}

void *rcvThread(void *arg) {
    server_info *server = (server_info *) arg;
    int *sockfd = server->sockfd;

    char *buf = malloc(LINE_SIZE+1);
    socklen_t server_len;
    server_len = sizeof(server->IPv4);

    int recv;
    while ((recv = recvfrom(*sockfd, buf, LINE_SIZE, 0, (struct sockaddr *)(server->IPv4), &server_len)) >= 0)
        fprintf(stdout, "Received message from server: %s\n", buf);

    if (recv == -1)
        fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);

    free(buf);
    pthread_exit(NULL);
}