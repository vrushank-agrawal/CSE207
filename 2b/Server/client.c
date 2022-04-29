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

#include "client.h"

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

    // int valid_add = check_add(dest_add); // self-inspiration // needless

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

    // from Moodle
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Socket, %s - %d\n", strerror(errno), errno);
        return 1;
    }

    int sent = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&IPv4, sizeof(IPv4)); 
    // idea from https://www.ibm.com/docs/en/zos/2.2.0?topic=programs-c-socket-udp-server
    
    if (sent < 0) {
        fprintf(stderr, "Error in sending data via UDP: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        return 1;
    }

    close(sockfd);
    return 0;
}



// custom function defined to check validity of an IPv4 address
// but sadly inet_pton already does that much better :(
int check_add (char *dest_add) {

    int len = strlen(dest_add);
    if (len < 7 || len > 15) {
        fprintf(stderr, "Invalid IPv4 address: Too long\n");
        return 1;
    } // too long to be a valid IPv4 address

    char *ptr = dest_add;
    int dots = 0, word_len = 0;

    while (*ptr != '\n') {
        if (word_len > 3) {
            fprintf(stderr, "Invalid IPv4 address has value > 255\n");
            return 1;
        } // check if number is < 255

        if (*ptr == '.') {
            dots++;
            ptr++;
            word_len = 0;
            continue;
        } // skip if a dot is encountered

        if (*ptr >= '0' || *ptr <= '9') {
            word_len++;
            ptr++;
            continue;
        } // check each character to be a number

        fprintf(stderr, "Invalid character %c in IPv4 address\n", *ptr);
        return 1;
    } // go through the entire string

    if (dots != 3) {
        fprintf(stderr, "Invalid IPv4 address with %d dots should be 3\n", dots);
        return 1;
    } // check number of dots

    return 0;
}