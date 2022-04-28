#ifndef SERVER_H
#define SERVER_H

// define DEBUG mode for program debugging
#define DEBUG
// longest string can have 9 filled positions i.e. 9*3 + 2 = 29 bytes
#define LINE_SIZE 29    
#define NTHREADS 30
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

int update_move(void *buf, char player);

/**
 * \brief checks if game ended
 * \return 0 if draw, 3 if game not over, else player number
 */

char check_win();

/**
 * \brief check if Game has ended
 * \param winner is winner info
 */

void game_end(char winner);

/**
 * \brief send FYI message
 * \param p is player to send info to
 */

void send_FYI(int p);

/**
 * \brief send MYM message
 * \param p is player to send info to
 */

void send_MYM(int p);

/**
 * \brief send END message
 * \param is winner info
 * \param p is player to send info to
 */

void send_END(char winner, int p);

/**
 * \brief send welcome message
 */

void welcome_fun();

/**
 * \brief sends connection reject message
 * \param arg
 */

void *rejectThread(void *arg);

/**
 * \brief checks status of game
 * \param arg is the message from client
 */

void *game_state(void *arg);

/**
 * \brief checks if client req is valid
 * \param arg
 */

void *decide_response(void *arg);

#endif // SERVER_H