#ifndef CLIENT_H
#define CLIENT_H

// define DEBUG mode
// #define DEBUG
// define DEBUG for function debugging
#define DEBUG_FUNC 
// longest input string decided by server
#define LINE_SIZE 26
#define INIT_MSG 7
#define MYM_SIZE 3
typedef struct sockaddr_in sockaddr_in;

/**
 * \brief struct to send data to server
 * \param contains message
 * \param server address struct
 */

typedef struct server_info {
    int *sockfd;
    char *msg;
    sockaddr_in *IPv4;
} server_info;

/**
 * \brief send data to server
 * \param destination server address
 * \param destination server port.
 * \return 0 on succes else error code.
 */

int send_UDP(char *dest_add, char *dest_port);

/**
 * \brief get position for next move
 * \return char array of MOV message
 */

char *get_coord();

/**
 * \brief draw the tic-tac-toe board
 * \param array with info of filled blocks
 * \param number of filled positions
 */

void draw_board(char * board_info, char positions);

/**
 * \brief receives data from server forever
 * \param argument passed to thread
 */

void *rcvThread(void *arg);

/**
 * \brief sends data to server
 * \param argument passed to thread
 */

void *sendThread(void *arg);

#endif // CLIENT_H