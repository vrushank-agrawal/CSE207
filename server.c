#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "server.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
char board_state[9];                        // 3x3 grid where 0x00 means box empty
char board_info[27];                        // we can have maximum 9 moves before a player wins
char moves = 0x00;                          // number of total moves made
int game_finish = 0;                        // True if game_just finished
sockaddr_in server;                         // defined globally for function access
socklen_t server_len = sizeof(server);
int sockfd, nbclients = 0, player_num = 1;
sockaddr client[2], rcv_client;
socklen_t client_len = sizeof(rcv_client);
pthread_t game, thread[NTHREADS];

int main (int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Missing argument. Please enter the PORT_NUMBER.\n");
        return 1;
    } // make sure all inputs are available
    int a; for(a=0;a<9;a++) board_state[a]=0x00;    // set board state
    char *port = argv[1];       // assign values
    return recv_UDP(port);      // receive UDP message;
}

int recv_UDP (char *dest_port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Socket, %s - %d\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }
    int port = strtol(dest_port, NULL, 10);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        fprintf(stderr, "Error in binding connection: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Sock binded. Waiting ...\n");
    void *buf = malloc((LINE_SIZE+1) * sizeof(char));
    void *temp_msg = malloc((LINE_SIZE+1) * sizeof(char));
    int recvd;
    sockaddr temp;
    socklen_t temp_len = sizeof(temp);

    while (1) {
        memset(temp_msg, 0, sizeof(*temp_msg));
        recvd = recvfrom(sockfd, temp_msg, LINE_SIZE, 0, &temp, &temp_len);
        if (recvd < 0) {
            fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        memcpy(buf, temp_msg, LINE_SIZE);           // send msg with diff address
        rcv_client = temp;                          // keep client with diff address
        if (pthread_create(&thread[0], NULL, decide_response, buf) != 0 ) {
            fprintf(stderr, "Error in creating respose deciding thread: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            exit(EXIT_FAILURE);
        }   // process the message
    }   // main while loop to listen to clients
    free(buf);
    free(temp_msg);
    close(sockfd);
    return 0;
}   // recv_UDP function


void *decide_response(void *arg) {
    int reject_client = 1;
    
    if (nbclients==2) {     // if client already exists then send reject message
        int j;
        sockaddr_in *copy_client, *temp;
        copy_client = (sockaddr_in *) &rcv_client;
        for (j=0;j<2;j++) {
            temp = (sockaddr_in *)&client[j];
            if ( copy_client->sin_port == temp->sin_port && copy_client->sin_addr.s_addr == temp->sin_addr.s_addr ) {
                reject_client = 0;
                if (pthread_create(&game, NULL, game_state, arg) != 0 ) {
                    fprintf(stderr, "Error in creating game thread: %s :%d\n", strerror(errno), errno);
                    close(sockfd);
                    exit(EXIT_FAILURE);
                }
                pthread_exit(NULL);
            }
        }   // client exists
    } else reject_client = 0;   // client does not exist
    
    if (reject_client) {
        if (pthread_create(&thread[0], NULL, rejectThread, NULL) != 0 ) {
            fprintf(stderr, "Error in creating responding thread: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            exit(EXIT_FAILURE);
        }   // reject the client
    } else {  // create new client
        if (!nbclients) {
            client[0] = rcv_client;
            printf("Client 1 assigned\n");
            welcome_fun();
        } else if (nbclients == 1) {
            client[1] = rcv_client;
            printf("Client 2 assigned\n");
            welcome_fun();
            send_FYI(0);        // start game by sending FYI to first player
            send_MYM(0);        // ask for MOV
        }
        nbclients++;
    }
    pthread_exit(NULL);
}   // function decide_response


void *game_state(void *arg) {
    void *buf = arg;   // msg from client
    player_num++;
    player_num%=2;              // interchange players
   
    if (*(char *)buf == 0x05) {
        #ifdef DEBUG
        printf("[DEBUG] MOV Message received\n");
        #endif
        char player;
        if (player_num==1) player = 0x02;
        else player = 0x01;
        if (update_move(buf+1, player)) {
            memcpy(board_info+(moves*3), &player, 1);      // update player number
            memcpy(board_info+(moves*3)+1, buf+1, 2);      // update player move
            moves++;                                       // update total moves
            if (moves > 4) {   // check if player won
                char winner;
                if ((winner = check_win()) != 0x03) {
                    #ifdef DEBUG
                    printf("[DEBUG] Winner Declared: %i in Moves: %i\n", winner, moves);
                    #endif
                    game_end(winner);    // End the game
                    pthread_exit(NULL);
                }
            }
            send_FYI(player%2);         // sends to next player
            send_MYM(player%2);         // sends to next player
        } else {    // otherwise send TXT
            memset(buf, 0, sizeof(*buf));                   // set for TXT message
            memcpy(buf, "SInvalid Move!\0", 16);
            memset(buf, 0x04, 1);
            if (sendto(sockfd, buf, 17, 0, &client[player_num], client_len) < 0) {
                fprintf(stderr, "Error in sending 'Invalid Move' TXT message: %s :%d\n", strerror(errno), errno);
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            #ifdef DEBUG
            printf("[DEBUG] Invalid MOV message sent\n");
            #endif
            send_MYM(player_num);       // send MYM message            
            player_num++;
            player_num%=2;              // reset player
        }   // if statement for update_move message
    }
    pthread_exit(NULL);
}


char check_win(){
    #ifdef DEBUG
    printf("[DEBUG] Entered check_win()\n");
    #endif
    int i;
    for (i=0;i<3;i++) {                                 // check for match in a row
        if (board_state[3*i] != 0x00)
            if (board_state[3*i] == board_state[3*i+1])
                if (board_state[3*i+1] == board_state[3*i+2]) {
                    #ifdef DEBUG
                    printf("[DEBUG] Winner Announced Line 193\n");
                    #endif
                    return board_state[3*i];            // return player number
                }
    }
    #ifdef DEBUG
    printf("[DEBUG] Entered Col check()\n");
    #endif
    for (i=0;i<3;i++) {                                 // check for match in a col
        if (board_state[i] != 0x00)
            if (board_state[i] == board_state[3+i])
                if (board_state[3+i] == board_state[6+i]) {
                    #ifdef DEBUG
                    printf("[DEBUG] Winner Announced Line 202\n");
                    #endif
                    return board_state[i];              // return player number
                }
    }
    #ifdef DEBUG
    printf("[DEBUG] Entered Diag1 Check\n");
    #endif
    // check for match in first diagonal
    if (board_state[0] != 0x00)
        if (board_state[0] == board_state[4])
            if (board_state[4] == board_state[8]){
                #ifdef DEBUG
                printf("[DEBUG] Winner Announced Line 211\n");
                #endif
                return board_state[0];                  // return player number
            }
    // check for match in second diagonal
    #ifdef DEBUG
    printf("[DEBUG] Entered Diag2 check\n");
    #endif
    if (board_state[2] != 0x00)
        if (board_state[2] == board_state[4])
            if (board_state[4] == board_state[6]) {
                #ifdef DEBUG
                printf("[DEBUG] Winner Announced Line 219\n");
                #endif
                return board_state[2];                  // return player number
            }
    #ifdef DEBUG
    printf("[DEBUG] Moves: %i\n", moves);
    #endif
    if (moves == 0x09) return 0x00;
    return 0x03;
}

int update_move(void *buf, char player) {
    char row = *(char *)(buf+1);
    char col = *(char *)(buf);
    #ifdef DEBUG
    printf("[DEBUG] Entered update_move()\n");
    printf("[DEBUG] row: %i col: %i\n", row, col);
    printf("[DEBUG] Board before update: ");
    int i=0; for(i=0;i<9;i++) printf("%d", board_state[i]);
    printf("\n");
    #endif
    if ((row>=0 && row<3) && (col>=0 && col<3)) {
        if (board_state[(3*row)+col] == 0x00) {  // check if position is empty
            board_state[(3*row)+col] = player;
            #ifdef DEBUG
            printf("[DEBUG] Board after update: ");
            for(i=0;i<9;i++) printf("%d", board_state[i]);
            printf("\n");
            #endif
            return 1;       // return success
        }
    }
    #ifdef DEBUG
    printf("[DEBUG] Board did not update: ");
    for(i=0;i<9;i++) printf("%d", board_state[i]);
    printf("\n");
    #endif
    return 0;               // return failure
}

void game_end(char winner) {
    // Send final state of board
    send_FYI(0);
    send_FYI(1);
    // Declare Winner
    send_END(winner, 0);
    send_END(winner, 1);
    
    int i;
    for(i=0;i<9;i++) board_state[i] = 0x00;     // reset board
    moves = 0x00;                               // reset moves
    nbclients = 0;                              // reset Num of clients
    player_num = 1;                             // reset player num
    memset(&client[0], 0, sizeof(sockaddr));    // reset client 1
    memset(&client[1], 0, sizeof(sockaddr));    // reset client 2
    printf("Game ended, waiting for new connections\n");
}

void send_MYM(int p) {
    void *mym = malloc(sizeof(char *));
    memset(mym, 0x02, 1);
    if (sendto(sockfd, mym, 1, 0, &client[p], client_len) < 0) {
        fprintf(stderr, "Error in sending MYM message: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    free(mym);
    #ifdef DEBUG
    printf("[DEBUG] MYM Message sent to player %d\n", p);
    #endif
}

void send_FYI(int p) {
    void *fyi = malloc(LINE_SIZE*sizeof(char *));
    memset(fyi, 0x01, 1);
    memset(fyi+1, moves, 1);
    if (moves) memcpy(fyi+2, board_info, moves*3);     // concatenate info of moves
    if (sendto(sockfd, fyi, (moves*3)+2, 0, &client[p], client_len) < 0) {
        fprintf(stderr, "Error in sending MYM message: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    free(fyi);  
    #ifdef DEBUG
    printf("[DEBUG] FYI Message sent to player %d\n", p);
    printf("[DEBUG] Board info sent: ");
    int i; for(i=0;i<(moves*3);i++) printf("%d", board_info[i]);
    printf("\n");
    #endif  
}

void send_END(char winner, int p) {
    void *end = malloc(2*sizeof(char *));
    memset(end, 0x03, 1);
    memset(end+1, winner, 1);
    if (sendto(sockfd, end, 2, 0, &client[p], client_len) < 0) {
        fprintf(stderr, "Error in sending END message: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    free(end);  
    #ifdef DEBUG
    printf("[DEBUG] END Msg sent to P: %i with Winner: %i\n", p, winner);
    #endif  
}

void welcome_fun() {
    #ifdef DEBUG
    printf("[DEBUG] Entered welcome_fun()\n");
    #endif
    void *welcome_msg = malloc(20*sizeof(char));
    memcpy(welcome_msg, "WWelcome Player x!\0", 20);
    memset(welcome_msg, 0x04, 1);
    char temp = 'X';                // to put player number
    if (nbclients == 1) temp = 'O';
    memset(welcome_msg+16, temp, 1);
    if (sendto(sockfd, welcome_msg, 20, 0, &client[nbclients], client_len) < 0) {
        fprintf(stderr, "Error in sending Welcome message: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    free(welcome_msg);
    #ifdef DEBUG
    printf("[DEBUG] Sent Welcome message to a client\n");
    #endif
}

void *rejectThread (void *arg) {
    #ifdef DEBUG
    printf("[DEBUG] Entered rejectThread()\n");
    #endif
    // create END message
    void *reject_msg = malloc(20*sizeof(char));
    memset(reject_msg, 0x03, 1);
    memset(reject_msg+1, 0xFF, 1);
    if (sendto(sockfd, reject_msg, 2, 0, &rcv_client, client_len) < 0) {
        fprintf(stderr, "Error in sending Rejection message: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    free(reject_msg);
    #ifdef DEBUG
    printf("[DEBUG] Sent rejection message to a client\n");
    #endif
    pthread_exit(NULL);
}
