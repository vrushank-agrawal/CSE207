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

    int recv, responsding = 0;            // variable to close response thread
    pthread_t thread;
    void *buf = malloc(LINE_SIZE+1);
    void *send_msg = malloc(LINE_SIZE+1);
    memcpy(buf, "HHello\0", 8);
    memset(buf, 0x04, 1);           // Set type of message to TXT
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
        memcpy(send_msg, buf, LINE_SIZE);
        #ifdef DEBUG_F
        printf("[DEBUG] Received message from server\n");
        printf("[DEBUG] buf received: ");
        char i; for (i=0x00;i<LINE_SIZE;i++) printf("%i", *(char *)(buf+i));
        printf("\n[DEBUG] send_msg received: ");
        for (i=0x00;i<LINE_SIZE;i++) printf("%i", *(char *)(send_msg+i));
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
    void *buf = arg;
    int i;
    #ifdef DEBUG
    printf("[DEBUG] Received message from server\n");
    printf("%i\n", *(char *)(buf));
    #endif
    if (*(char *)(buf) == 0x03) {   // END
        #ifdef DEBUG
        printf("[DEBUG] Received END message from server\n");
        printf("%i\n", *(unsigned char *)(buf+1));
        #endif
        if (*(unsigned char *)(buf+1) == 0xFF)
            fprintf(stderr, "No player spot found. Server denied access.\n");
        else if (*(char *)(buf+1) == 0x01)
            printf("Player X won the game!\n");
        else if (*(char *)(buf+1) == 0x02)
            printf("Player O won the game\n");
        else if (*(char *)(buf+1) == 0x00)
            printf("The game has ended in a draw\n");
        else
            printf("unexpected behaviour !! APOCALYPSE !!\n");
        close(sockfd);
        exit(EXIT_SUCCESS);

    } else if (*(char *)(buf) == 0x01) {  // FYI
        #ifdef DEBUG
        char pos = *(char *)(buf+1);
        printf("[DEBUG] Received FYI message from server\n");
        printf("[DEBUG] buf received: ");
        char i; for (i=0x00;i<(3*pos)+2;i++) printf("%i", *(char *)(buf+i));
        printf("\n");
        #endif
        printf("%i filled positions:\n", *(char *)(buf+1));
        draw_board(buf + 2, *(char *)(buf+1));

    } else if (*(char *)(buf) == 0x02){   // MYM
        char *msg = get_coord(); // get coord to play
        #ifdef DEBUG
        printf("[DEBUG] Received MYM message from server\n");
        printf("zero: %d\n", msg[0]);
        printf("first: %d\n", msg[1]);
        printf("second: %d\n", msg[2]);
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
    } else if (*(char *)(buf) == 0x04) {    // TXT
        #ifdef DEBUG
        printf("[DEBUG] Received TXT message from server\n");
        #endif
        i = 1;
        char *msg = (char *)malloc((strlen((char *)buf)-1)*sizeof(char)); 
        while (*(char *)(buf+i) != '\0') {
            msg[i-1] = *(char *)(buf+i);
            i++;
        }
        msg[i] = '\0';
        printf("Server says: %s\n", msg);
        free(msg);
        
    } else if (*(char *)(buf) == 0x06) {    // LTI
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
    char *m = malloc(3 * sizeof(char));    
    memset(m, 0x05, 1);
    int i, n;
    while (1){
        printf("Enter Column:");
        n=scanf("%d", &i);
        #ifdef DEBUG
        printf("%d\n", i);
        #endif
        if (n==1) {
            if (i==0 || i==1 || i==2) break;
            else printf("Invalid Col Number. Please enter between 0 & 2\n");
        }
        // fflush(stdin);
    }
    if (i==0) m[1] = 0x00;
    else if (i==1) m[1] = 0x01;
    else m[1] = 0x02;
    while (1){
        printf("Enter Row:");
        n=scanf("%d", &i);
        #ifdef DEBUG
        printf("%d\n", i);
        #endif
        if (n==1) {
            if (i==0 || i==1 || i==2) break;
            else printf("Invalid Row Number. Please enter between 0 & 2\n");
        }
        // fflush(stdin);
    }
    if (i==0) m[2] = 0x00;
    else if (i==1) m[2] = 0x01;
    else m[2] = 0x02;
    #ifdef DEBUG
    printf("[DEBUG] Get_Coord() Col: %i Row: %i\n", m[1], m[2]);
    #endif
    return m;
}

void draw_board(void *buf, char pos) {
    char i = 0x00;
    char board[9] = "         ";        // 3x3 entries of tic tac toe are initialized to be empty
    #ifdef DEBUG
    printf("[DEBUG] Entered draw_board()\n");
    printf("[DEBUG] Board info received: ");
    for (i=0x00;i<3*pos;i++) printf("%i", *(char *)(buf+i));
    printf("\n");
    #endif
    for (i=0x00;i<pos;i++){
        char player = *(char *)(buf+(3*i));              // number of player is either 1 or 2
        char col = *(char *)(buf+((3*i)+1));          
        char row = *(char *)(buf+((3*i)+2));          
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