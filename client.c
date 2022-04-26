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
sockaddr_in IPv4;
socklen_t server_len;
int sockfd;
int ended = 0;

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
        exit(0);
    }

    #ifdef DEBUG
    printf("[DEBUG] Sock created\n");
    #endif

    char *msg = malloc(INIT_MSG * sizeof(char));    // assign message string for hello;
    char *buf = malloc(LINE_SIZE+1);
    char *send_msg = malloc(LINE_SIZE+1);
    server_len = sizeof(IPv4);
    strncpy(msg, "HHello\0", INIT_MSG);
    msg[0] = 0x04; // Set type of message to TXT

    if (sendto(sockfd, msg, LINE_SIZE, 0, (struct sockaddr *)&IPv4, server_len) < 0) {
        fprintf(stderr, "Error in sending data: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        free(buf);
        free(send_msg);
        return 1;
    }
    free(msg);

    #ifdef DEBUG
    printf("[DEBUG] Sent Hello Message\n");
    #endif

    int recv;
    if ((recv = recvfrom(sockfd, buf, LINE_SIZE, 0, (struct sockaddr *)&IPv4, &server_len)) == -1){
        fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);
        free(buf);
        free(send_msg);
        close(sockfd);
        return 1;
    }

    // // sample fyi instruction
    // strcpy(buf, "04111212122231"); 
    // buf[0] = 0x01;

    // // sample TXT instruction
    // strcpy(buf, "0Hello Back\0"); 
    // buf[0] = 0x04;

    // // sample end instruction
    // buf[0] = 0x03;
    // buf[1] = 0xFF;
    
    int i = 1;
    char *welcome = (char *)malloc((strlen((char *)buf)-1)*sizeof(char)); 
    while (buf[i] != '\0') {
        welcome[i-1] = buf[i];
        i++;
    }
    welcome[++i] = '\0';
    printf("Message from server: %s\n", welcome);
    free(welcome);

    int responsding = 0;            // variable to close response thread
    pthread_t thread;

    while (1) {
        memset(buf, 0, sizeof(*buf));
        // memset(send_msg, 0, sizeof(*send_msg));
        if ((recv = recvfrom(sockfd, buf, LINE_SIZE, 0, (struct sockaddr *)&IPv4, &server_len)) == -1){
            fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);
            free(buf);
            free(send_msg);
            close(sockfd);
            exit(0);
        }

        // join thread if running
        if (responsding)
            if (pthread_join(thread, NULL) != 0) {
                fprintf(stderr, "Error in joining responding thread: %s :%d\n", strerror(errno), errno);
                free(buf);
                free(send_msg);
                close(sockfd);
                exit(0);
            }

        strncpy(send_msg, buf, LINE_SIZE);
        if (pthread_create(&thread, NULL, play_game, send_msg) != 0 ) {
            fprintf(stderr, "Error in creating responding thread: %s :%d\n", strerror(errno), errno);
            free(buf);
            free(send_msg);
            close(sockfd);
            exit(0);
        }
        responsding = 1; // close thread in loop
        
        if (ended) break;
    }
    
    // close the message receiving thread from server
    if (pthread_join(thread, NULL) != 0) {
        fprintf(stderr, "Error in joining receive thread: %s :%d\n", strerror(errno), errno);
        free(buf);
        free(send_msg);
        close(sockfd);
        exit(0);
    }

    free(buf);
    free(send_msg);
    close(sockfd);
    return 0;
}

void *play_game(void *arg) {

    char *buf = (char *)arg;
    int i;

    if (buf[0] == 0x03) {   // END
        ended = 1;              // game has ended
        if (buf[1] == 0xFF){
            fprintf(stderr, "No player spot found. Server denied access.\n");
            close(sockfd);
            free(buf);
            pthread_exit(NULL);
        } else if (buf[1] == '1')
            printf("Player X won the game!\n");
        else if (buf[1] == '2')
            printf("Player O won the game\n");
        else if (buf[1] == '0')
            printf("The game has ended as a draw\n");
        else
            printf("unexpected behaviour !! APOCALYPSE !!");
    }
    
    else if (buf[0] == 0x01) {  // FYI
        #ifdef DEBUG
        printf("[DEBUG] Received FYI message from server\n");
        #endif

        #ifdef DEBUG_FUNC
        printf("%s\n", buf);
        #endif

        printf("%c filled poisitons:\n", *(buf+1));
        draw_board(buf + 2, buf[1]);
    }

    else if (buf[0] == 0x02){   // MYM
        #ifdef DEBUG
        printf("[DEBUG] Received MYM message from server\n");
        #endif

        char *msg = get_coord(); // get coord to play
        if (sendto(sockfd, msg, LINE_SIZE, 0, (struct sockaddr *)&IPv4, sizeof(IPv4)) < 0) {
            fprintf(stderr, "Error in sending data: %s :%d\n", strerror(errno), errno);
            free(buf);
            free(msg);
            close(sockfd);
            exit(0);
        }
        free(msg);

        #ifdef DEBUG
        printf("[DEBUG] Sent MYM message to server\n");
        #endif

    } else if (buf[0] == 0x04) {    // TXT

        #ifdef DEBUG
        printf("[DEBUG] Received TXT message from server\n");
        #endif

        i = 1;
        char *msg = (char *)malloc((strlen((char *)buf)-1)*sizeof(char)); 
        while (buf[i] != '\0') {
            msg[i-1] = buf[i];
            i++;
        }
        msg[++i] = '\0';
        printf("Message from server: %s\n", msg);
        free(msg);
        
    } else if (buf[0] == 0x06) {    // LTI
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
    char *msg = (char *)malloc(MYM_SIZE * sizeof(char));
    msg[0] = 0x05;
    while (1){
        printf("Column number: ");
        scanf(" %c", msg+1);
        if ((msg[1] == '1' || msg[1] == '2' || msg[1] == '3'))
            break;
        else
            printf("Invalid Col Number. Please enter between 1 & 3 without spaces\n");
    }
    while (1){
        printf("Row number: ");
        scanf(" %c", msg+2);
        if (msg[2] == '1' || msg[2] == '2' || msg[2] == '3')
            break;
        else
            printf("Invalid Row Number. Please enter between 1 & 3\n");
    }
    printf("Your Move - COL: %c - ROW: %c\n", msg[1], msg[2]);
    return msg;
}

void draw_board(char *buf, char positions) {

    #ifdef DEBUG
    printf("[DEBUG] Entered draw_board()\n");
    #endif

    #ifdef DEBUG_FUNC
    printf("[DEBUG_FUNC] board_info: %s\n", buf);
    #endif

    int i = 0;
    int pos = atoi(&positions);
    char board[9] = "         ";        // 3x3 entries of tic tac toe are initialized to be empty
    for (i=0;i<pos;i++){
        char p = buf[3*i];              // number of player is either 1 or 2
        int player = atoi(&p);
        char c = buf[(3*i)+1];          // column number - 1 to align with zero
        int col = (int)(c)-49;          // idk weird arithemtics
        char r = buf[(3*i)+2];          // row number -1 to align with zero
        int row = (int)(r)-49;          // idk weird arithmetics
        if (player == 1)
            board[3*row+col] = 'X';     // if player 1 then we mark X
        else
            board[3*row+col] = 'O';     // if player 2 then we mark O
    }
    // now we simply draw the tic-tac-toe board
    printf("|---+---+---|\n");
    for (i=0;i<3;i++){
        printf("| %c | %c | %c |\n", board[3*i], board[3*i+1], board[3*i+2]);
        printf("|---+---+---|\n");
    }
}