#ifndef CLIENT_H
#define CLIENT_H

/**
 * \brief send data to server
 * \param destination server address
 * \param destination server port.
 * \return 0 on succes else error code.
 */
int send_UDP(char *dest_add, char *dest_port, char *msg);

#endif // CLIENT_H