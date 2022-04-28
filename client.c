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
sockaddr_in IPv4;           // defined globally for function access
socklen_t server_len = sizeof(IPv4);
int sockfd;

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
    IPv4.sin_family = AF_INET;
    IPv4.sin_port = htons(port);

    if (inet_pton(AF_INET, dest_add, &IPv4.sin_addr) <= 0) { // convert server address to network readable
        fprintf(stderr, "Invalid IPv4 address\n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Socket, %s - %d\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    #ifdef DEBUG
    printf("[DEBUG] Sock created\n");
    #endif

    int recv, responsding = 0;            // variable to close response thread
    pthread_t thread;
    char *buf = malloc(LINE_SIZE+1);
    char *send_msg = malloc(LINE_SIZE+1);
    strncpy(buf, "HHello\0", 8);
    buf[0] = 0x04; // Set type of message to TXT

    if (sendto(sockfd, buf, LINE_SIZE, 0, (struct sockaddr *)&IPv4, server_len) < 0) {
        fprintf(stderr, "Error in sending data: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        exit(0);
    }

    #ifdef DEBUG
    printf("[DEBUG] Sent Hello Message\n");
    #endif

    while (1) {
        memset(buf, 0, sizeof(*buf));
        // memset(send_msg, 0, sizeof(*send_msg));
        if ((recv = recvfrom(sockfd, buf, LINE_SIZE, 0, (struct sockaddr *)&IPv4, &server_len)) == -1){
            fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // join thread if running
        if (responsding)
            if (pthread_join(thread, NULL) != 0) {
                fprintf(stderr, "Error in joining responding thread: %s :%d\n", strerror(errno), errno);
                close(sockfd);
                exit(EXIT_FAILURE);
            }

        strncpy(send_msg, buf, LINE_SIZE);
        
        #ifdef DEBUG_F
        printf("[DEBUG] Received message from server\n");
        printf("[DEBUG] buf received: ");
        char i; for (i=0x00;i<LINE_SIZE;i++) printf("%i", *(buf+i));
        printf("\n[DEBUG] send_msg received: ");
        for (i=0x00;i<LINE_SIZE;i++) printf("%i", *(send_msg+i));
        printf("\n");
        #endif

        if (pthread_create(&thread, NULL, game_response, send_msg) != 0 ) {
            fprintf(stderr, "Error in creating responding thread: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        responsding = 1; // close thread in loop
    } // end while loop
    
    free(buf);
    free(send_msg);
    close(sockfd);
    return 0;
}

void *game_response(void *arg) {
    char *buf = (char *)arg;
    int i;

    #ifdef DEBUG
    printf("[DEBUG] Received message from server\n");
    printf("%i\n", *(buf));
    #endif
    
    if (buf[0] == 0x03) {   // END
        unsigned char *buf = (unsigned char *)arg;
        
        #ifdef DEBUG
        printf("[DEBUG] Received END message from server\n");
        printf("%i\n", *(buf+1));
        #endif

        if (*(buf+1) == 0xFF)
            fprintf(stderr, "No player spot found. Server denied access.\n");
        else if (*(buf+1) == 0x01)
            printf("Player X won the game!\n");
        else if (*(buf+1) == 0x02)
            printf("Player O won the game\n");
        else if (*(buf+1) == 0x00)
            printf("The game has ended in a draw\n");
        else
            printf("unexpected behaviour !! APOCALYPSE !!\n");
        close(sockfd);
        exit(EXIT_SUCCESS);

    } else if (*(buf) == 0x01) {  // FYI
        char pos = *(buf+1);
        
        #ifdef DEBUG
        printf("[DEBUG] Received FYI message from server\n");
        printf("[DEBUG] buf received: ");
        char i; for (i=0x00;i<(3*pos)+2;i++) printf("%i", *(buf+i));
        printf("\n");
        #endif

        printf("%i filled positions:\n", *(buf+1));
        draw_board(buf + 2, *(buf+1));

    } else if (*(buf) == 0x02){   // MYM
        char *msg = get_coord(); // get coord to play
        
        #ifdef DEBUG
        printf("[DEBUG] Received MYM message from server\n");
        printf("[DEBUG] MSG SENT - COL: %i - ROW: %i\n", msg[1], msg[2]);
        #endif

        if (sendto(sockfd, msg, LINE_SIZE, 0, (struct sockaddr *)&IPv4, sizeof(IPv4)) < 0) {
            fprintf(stderr, "Error in sending MOV data: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        free(msg);

        #ifdef DEBUG
        printf("[DEBUG] Sent MOV message to server\n");
        #endif

    } else if (*(buf) == 0x04) {    // TXT
        #ifdef DEBUG
        printf("[DEBUG] Received TXT message from server\n");
        #endif

        i = 1;
        char *msg = (char *)malloc((strlen((char *)buf)-1)*sizeof(char)); 
        while (*(buf+i) != '\0') {
            msg[i-1] = *(buf+i);
            i++;
        }
        msg[i] = '\0';
        printf("Server says: %s\n", msg);
        free(msg);
        
    } else if (*(buf) == 0x06) {    // LTI
        #ifdef DEBUG
        printf("[DEBUG] Received LTI message from server ??\n");
        #endif
    }
    pthread_exit(NULL);
}

char *get_coord(){
    #ifdef DEBUG
    printf("[DEBUG] Entered get_coord()\n");
    #endif

    printf("Server has asked for your move\n");

    char *msg = malloc(3 * sizeof(char));    
    msg[0] = 0x05;
    while (1){
        printf("Enter Column: ");
        scanf(" %c", msg+1);
        if (msg[1] == '0' || msg[1] == '1' || msg[1] == '2')
            break;
        else
            printf("Invalid Col Number. Please enter between 0 & 2\n");
    }
    while (1){
        printf("Enter Row: ");
        scanf(" %c", msg+2);
        if (msg[2] == '0' || msg[2] == '1' || msg[2] == '2')
            break;
        else
            printf("Invalid Row Number. Please enter between 0 & 2\n");
    }

    if (msg[1]=='0') msg[1]=0x00; 
    else if (msg[1]=='1') msg[1]=0x01;
    else msg[1]=0x02;
    if (msg[2]=='0') msg[2]=0x00;
    else if (msg[2]=='1') msg[2]=0x01;
    else msg[2]=0x02;
    return msg;
}

void draw_board(char *buf, char pos) {
    #ifdef DEBUG
    printf("[DEBUG] Entered draw_board()\n");
    #endif

    char i = 0x00;
    char board[9] = "         ";        // 3x3 entries of tic tac toe are initialized to be empty
    
    #ifdef DEBUG
    printf("[DEBUG] Board info received: ");
    for (i=0x00;i<3*pos;i++) printf("%i", *(buf+i));
    printf("\n");
    #endif

    for (i=0x00;i<pos;i++){
        char player = *(buf+(3*i));              // number of player is either 1 or 2
        char col = *(buf+((3*i)+1));          
        char row = *(buf+((3*i)+2));          
        if (player == 0x01)
            board[3*row+col] = 'X';     // if player 1 then we mark X
        else
            board[3*row+col] = 'O';     // if player 2 then we mark O

        #ifdef DEBUG
        printf("[DEBUG] p: %d col: %d row:%d \n", player, col, row);
        #endif
    }
    // now we simply draw the tic-tac-toe board
    printf("|---+---+---|\n");
    for (i=0;i<3;i++){
        printf("| %c | %c | %c |\n", board[3*i], board[3*i+1], board[3*i+2]);
        printf("|---+---+---|\n");
    }
}