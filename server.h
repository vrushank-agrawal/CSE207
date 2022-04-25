#ifndef SERVER_H
#define SERVER_H

// define DEBUG mode for program debugging
#define DEBUG
// define DEBUG for function debugging
#define DEBUG_FUNC 
// longest string can have 9 filled positions i.e. 9*3 + 2 = 26 bytes
#define LINE_SIZE 29    
// we can have max 2 clients and 1 thread to reject others
#define NTHREADS 3
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
 * \brief sets FYI message to be sent
 * \param buf that gives move coordinates from player
 * \param player gives the player number
 * \return 1 if valid move otherwise 0
 */

int update_move(char *buf, char player);

/**
 * \brief sets FYI message to be sent
 * \return 0 if draw, 3 if game not over, else player number
 */

char check_win();

/**
 * \brief define actions to be performed by the thread
 * \param arg is struct info_client
 * \return 0 on success else error code.
 */

void *welcomeThread(void *arg);

/**
 * \brief sends connection reject message
 * \param arg is the port number passed as char
 * \return 0 on success else error code.
 */

void *rejectThread(void *arg);

#endif // SERVER_H