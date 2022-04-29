#ifndef SERVER_H
#define SERVER_H

/**
 * \brief send data to server
 * \param destination server address
 * \param destination server port.
 * \return 0 on succes else error code.
 */
int recv_UDP(char *dest_port);

#endif // SERVER_H