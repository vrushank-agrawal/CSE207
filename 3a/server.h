#ifndef SERVER_H
#define SERVER_H

#define LINE_SIZE 80
#define NTHREADS 50

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;

/**
 * \brief struct to send data to server
 * \param contains sock int
 * \param contains msg le
 * \param server address struct
 * \param contains message
 */

typedef struct info_client {
    int *sockfd;
    int *recvd;
    sockaddr *client;
    char *msg;
} info_client;

/**
 * \brief send data to server
 * \param destination server address
 * \param destination server port.
 * \return 0 on succes else error code.
 */

int recv_UDP(char *dest_port);

/**
 * \brief define actions to be performed by the thread
 * \param argument passed to thread
 * \return 0 on success else error code.
 */

void *sockThread(void *arg);

#endif // SERVER_H