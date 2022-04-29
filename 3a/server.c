#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "server.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main (int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Missing argument. Please enter the PORT_NUMBER.\n");
        return 1;
    } // make sure all inputs are available

    char *port = argv[1];    // assign values
    return recv_UDP(port);     // receive UDP message;
}

int recv_UDP (char *dest_port) {

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Socket, %s - %d\n", strerror(errno), errno);
        return 1;
    }

    int port = strtol(dest_port, NULL, 10);
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

    pthread_t threads[NTHREADS];
    int i = 0;

    info_client *client = malloc(NTHREADS * sizeof(*client));
    char *buf = malloc((LINE_SIZE+1) * sizeof(char));

    while (1) {
        struct sockaddr rcv_client;
        socklen_t client_len;
        client_len = sizeof(rcv_client);

        int recvd = recvfrom(sockfd, buf, LINE_SIZE, 0, &rcv_client, &client_len);
        if (recvd < 0) {
            fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            pthread_exit(NULL);
        }

        // if client already exists then update message in its struct
        int j, found = 0;
        sockaddr_in *copy_client, *temp;
        copy_client = (sockaddr_in *) &rcv_client;
        for (j = 0; j<i; j++) {
            temp = (sockaddr_in *) client[j].client;
            if ( copy_client->sin_port == temp->sin_port && copy_client->sin_addr.s_addr == temp->sin_addr.s_addr ) {
                pthread_mutex_lock(&lock);
                memset(client[j].msg, 0, sizeof(*client[j].msg)); // reset previous message
                strncpy( client[j].msg, buf, recvd);
                *client[j].recvd = recvd;
                pthread_mutex_unlock(&lock);
                found = 1;
                break;
            }
        }
        
        // if new client then check thread availability and add client
        if (!found) {
            if (i < NTHREADS) {

                // create new client
                pthread_mutex_lock(&lock);
                client[i].sockfd = (int *)malloc(1 * sizeof(int));
                *client[i].sockfd = sockfd;
                client[i].recvd = (int *)malloc(1 * sizeof(int));
                *client[i].recvd = recvd;
                client[i].client = (sockaddr *)malloc(sizeof(sockaddr *));
                memcpy(client[i].client, &rcv_client, sizeof(sockaddr));
                client[i].msg = (char *)malloc(NTHREADS * sizeof(char));
                strcpy(client[i].msg, buf);
                pthread_mutex_unlock(&lock);
                
                // reset buf memory for the next input
                memset(buf, 0, sizeof(*buf));

                if ( pthread_create(&threads[i], NULL, sockThread, &client[i]) != 0 ) {
                    fprintf(stderr, "Error in creating thread: %s :%d\n", strerror(errno), errno);
                    close(sockfd);
                    return 1;
                }
                i++;

            } else { // wait to join all threads
                i = 0;
                while (i < NTHREADS)
                    if (pthread_join(threads[i++],NULL) != 0) {
                        fprintf(stderr, "Error in joining thread: %s :%d\n", strerror(errno), errno);
                        close(sockfd);
                        return 1;
                    }
                i = 0;
            }
        }
    }

    free(buf);
    int j;
    for (j=0; j<i; j++) {
        free(client[j].client);
        free(client[j].msg);
    }
    free(client);
    close(sockfd);
    return 0;
}


void *sockThread(void *arg) {
    info_client *client = (info_client *) arg;
    socklen_t client_len = sizeof(*(client -> client));

    while (1) {
        sleep(3); // output data after every 1 second

        pthread_mutex_lock(&lock); // lock thread for reading data
        if (sendto(*(client->sockfd), client->msg, *(client->recvd), 0, client->client, client_len) < 0) {
            fprintf(stderr, "Error in sending confirmation message: %s :%d\n", strerror(errno), errno);
            close(*(client->sockfd));
            pthread_mutex_unlock(&lock); // unlock thread as data processing finished
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&lock); // unlock thread as data processing finished
        // printf("client: %s\n", client->client->sa_data);
    }
    pthread_exit(NULL);
}