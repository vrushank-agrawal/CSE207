#ifndef CLIENT_H
#define CLIENT_H

// define DEBUG mode
#define DEBUG
// longest input string decided by server
#define LINE_SIZE 80
typedef struct sockaddr_in sockaddr_in;

/**
 * \brief send data to server
 * \param destination server address
 * \param destination server port.
 * \return 0 on succes else error code.
 */

int send_UDP(char *dest_add, char *dest_port);

/**
 * \brief get position for next move
 * \return int array of MOV message
 */

char *get_coord();

/**
 * \brief draw the tic-tac-toe board
 * \param array with info of filled blocks
 * \param number of filled positions
 */

void draw_board(char * board_info, char positions);

/**
 * \brief play game
 * \param argument passed to thread
 */

void *game_response(void *arg);

#endif // CLIENT_H