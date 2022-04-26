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
char board_state[9] = "000000000";             // 3x3 grid where 0 means box empty
int moves = 0;                              // number of total moves made
int game_finish = 0;                        // True if game_just finished

int main (int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Missing argument. Please enter the PORT_NUMBER.\n");
        return 1;
    } // make sure all inputs are available

    char *port = argv[1];    // assign values
    return recv_UDP(port);     // receive UDP message;
}

int recv_UDP (char *dest_port) {

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Socket, %s - %d\n", strerror(errno), errno);
        exit(EXIT_FAILURE);
    }

    int port = strtol(dest_port, NULL, 10);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    int binded = bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    if (binded < 0){
        fprintf(stderr, "Error in binding connection: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    #ifdef DEBUG
    printf("[DEBUG] Sock created and binded\n");
    #endif
    
    pthread_t threads[NTHREADS+1];
    int i = 0;

    info_client *client = malloc((NTHREADS+1) * sizeof(*client));   // 1 extra thread to reject clients
    char *buf = malloc((LINE_SIZE+1) * sizeof(char));
    int reject_client;

    while (1) {
        struct sockaddr rcv_client;
        socklen_t client_len;
        client_len = sizeof(rcv_client);

        int recvd = recvfrom(sockfd, buf, LINE_SIZE, 0, &rcv_client, &client_len);
        if (recvd < 0) {
            fprintf(stderr, "Error in receiving data via UDP: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            free(buf);
            pthread_exit(NULL);
        }
        reject_client = 1;

        #ifdef DEBUG
        printf("[DEBUG] Received initial message from a client\n");
        #endif

        // if client already exists then send reject message
        if (i==2) {
            int j, found = 0;
            sockaddr_in *copy_client, *temp;
            copy_client = (sockaddr_in *) &rcv_client;
            for (j = 0; j<i; j++) {
                temp = (sockaddr_in *) client[j].client;
                if ( copy_client->sin_port == temp->sin_port && copy_client->sin_addr.s_addr == temp->sin_addr.s_addr ) {
                    reject_client = 0;
                }
            }
        } else {
            reject_client = 0;
        }
        
        // create new client
        client[i].sockfd = (int *)malloc(1 * sizeof(int));
        *client[i].sockfd = sockfd;
        client[i].recvd = (int *)malloc(1 * sizeof(int));
        *client[i].recvd = recvd;
        client[i].client = (sockaddr *)malloc(sizeof(sockaddr *));
        memcpy(client[i].client, &rcv_client, sizeof(sockaddr));
        
        if (reject_client) {
            // create player message for client
            client[i].msg = (char *)malloc(NTHREADS * sizeof(char));
            strcpy(client[i].msg, "WWelcome Player x!\0");

            // // reset buf memory for the next input
            // memset(buf, 0, sizeof(*buf));

            // if ( pthread_create(&threads[i], NULL, sockThread, &client[i]) != 0 ) {
            //     fprintf(stderr, "Error in creating thread: %s :%d\n", strerror(errno), errno);
            //     close(sockfd);
            //     return 1;
            // }
            
        } else {
            // create player message for client
            client[i].msg = (char *)malloc(NTHREADS * sizeof(char));
            strcpy(client[i].msg, "WWelcome Player x!\0");
            client[i].msg[0] = 0x04;
            char temp = 'X';                // to put player number
            if (i == 1) temp = 'O';
            client[i].msg[16] = temp;
            
            if ( pthread_create(&threads[i], NULL, welcomeThread, &client[i]) != 0 ) {
                fprintf(stderr, "Error in creating welcome thread: %s :%d\n", strerror(errno), errno);
                exit(EXIT_FAILURE);
            }
            i++;

            #ifdef DEBUG
            printf("[DEBUG] client thread created\n");
            #endif
                
            if (i==2) { // start game

                // TODO
                // write code to listen to incoming client requests
                // if ( pthread_create(&threads[i], NULL, rejectThread, dest_port) != 0 ) {
                //     fprintf(stderr, "Error in creating Reject thread: %s :%d\n", strerror(errno), errno);
                //     exit(EXIT_FAILURE);
                // }

                #ifdef DEBUG
                printf("[DEBUG] Reject thread created\n");
                #endif

                // join previous threads
                if (pthread_join(threads[0],NULL) != 0) {
                    fprintf(stderr, "Error in joining welcome thread 0: %s :%d\n", strerror(errno), errno);
                    exit(EXIT_FAILURE);
                }
                if (pthread_join(threads[1],NULL) != 0) {
                    fprintf(stderr, "Error in joining welcome thread 1: %s :%d\n", strerror(errno), errno);
                    exit(EXIT_FAILURE);
                }    
                
                #ifdef DEBUG
                printf("[DEBUG] Client Threads joined\n");
                #endif

                socklen_t client_len = sizeof(client[0]);
                char board_info[27];                        // we can have maximum 9 moves before a player wins
                strcpy(board_info,"");
                int j = 1;
                while (1) {
                    j++;
                    j%=2;     // interchange players
                    
                    // set FYI message
                    memset(buf, 0, sizeof(*buf));                       
                    buf[0] = 0x01;
                    buf[1] = moves+'0';
                    if (buf[1]) strncpy(buf+2, board_info, moves*3);     // concatenate info of moves
                    if (sendto(*(client[j].sockfd), buf, LINE_SIZE+1, 0, client[j].client, client_len) < 0) {
                        fprintf(stderr, "Error in sending FYI message: %s :%d\n", strerror(errno), errno);
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    #ifdef DEBUG
                    printf("[DEBUG] FYI Message sent\n");
                    #endif

                    #ifdef DEBUG_FUNC
                    printf("[DEBUG_FUNC] BUF message sent:%s\n", buf);
                    #endif

                    // set MYM message
                    memset(buf, 0, sizeof(*buf));                       
                    buf[0] = 0x02;
                    if (sendto(*(client[j].sockfd), buf, 1, 0, client[j].client, client_len) < 0) {
                        fprintf(stderr, "Error in sending MYM message: %s :%d\n", strerror(errno), errno);
                        close(sockfd);
                        exit(EXIT_FAILURE);
                    }

                    #ifdef DEBUG
                    printf("[DEBUG] MYM Message sent\n");
                    #endif

                    // get valid MOV message
                    while (1) { 
                        memset(buf, 0, sizeof(*buf));                           // set for MOV message
                        int recvd = recvfrom(*(client[j].sockfd), buf, 3, 0, client[j].client, &client_len);
                        if (recvd < 0) {
                            fprintf(stderr, "Error in receiving MOV message: %s :%d\n", strerror(errno), errno);
                            close(sockfd);
                            exit(EXIT_FAILURE);
                        }
                        
                        #ifdef DEBUG
                        printf("[DEBUG] MOV Message received\n");
                        #endif
                        
                        if (buf[0] == 0x05) {
                            char player;
                            if (j==1) player = '2';
                            else player = '1';
                            if (update_move(buf+1, player)) {
                                // char update[3]; update[0]=player; update[1]=buf[1]; update[2]=buf[2];
                                strncpy(board_info+(moves*3), &player, 1);       // update player number
                                strncpy(board_info+(moves*3)+1, buf+1, 2);      // update player move
                                moves++;                                        // update total moves
                                break;                                          // if valid MOV then exit while loop
                            } else {    // otherwise send TXT until valid MOV received
                                sleep(1);
                                memset(buf, 0, sizeof(*buf));                   // set for TXT message
                                strcpy(buf, "SInvalid Move!\0");
                                buf[0] = 0x04;
                                if (sendto(*(client[j].sockfd), buf, 17, 0, client[j].client, client_len) < 0) {
                                    fprintf(stderr, "Error in sending 'Invalid Move' TXT message: %s :%d\n", strerror(errno), errno);
                                    close(sockfd);
                                    exit(EXIT_FAILURE);
                                }

                                #ifdef DEBUG
                                printf("[DEBUG] Invalid MOV message sent\n");
                                #endif

                            }   // if statement for update_move message
                        } else {
                            memset(buf, 0, sizeof(*buf));                   // set for TXT message
                            strcpy(buf, "SSend MOV message!\0");
                            buf[0] = 0x04;
                            if (sendto(*(client[j].sockfd), buf, 21, 0, client[j].client, client_len) < 0) {
                                fprintf(stderr, "Error in sending 'Invalid Move' TXT message: %s :%d\n", strerror(errno), errno);
                                close(sockfd);
                                exit(EXIT_FAILURE);
                            }
                            
                            #ifdef DEBUG
                            printf("[DEBUG] Not MOV Message sent\n");
                            #endif

                        }   // if statement to read MOV message
                    }   // while loop for MOV message

                    // check if game is finished
                    char winner;
                    if ((winner = check_win()) != '3') {

                        // set FYI message
                        memset(buf, 0, sizeof(*buf));                       
                        buf[0] = 0x01;
                        buf[1] = moves+'0';
                        if (buf[1]) strncpy(buf+2, board_info, moves*3);     // concatenate info of moves
                        if (sendto(*(client[j].sockfd), buf, LINE_SIZE+1, 0, client[j].client, client_len) < 0) {
                            fprintf(stderr, "Error in sending FYI message: %s :%d\n", strerror(errno), errno);
                            close(sockfd);
                            exit(EXIT_FAILURE);
                        }

                        #ifdef DEBUG
                        printf("[DEBUG] FYI Message sent\n");
                        #endif

                        #ifdef DEBUG_FUNC
                        printf("[DEBUG_FUNC] BUF message sent:%s\n", buf);
                        #endif

                        memset(buf, 0, sizeof(*buf));                       // set for END message
                        buf[0] = 0x03;
                        buf[1] = winner;
                        // send END message
                        if (sendto(*(client[0].sockfd), buf, 2, 0, client[0].client, client_len) < 0) {
                            fprintf(stderr, "Error in sending END message: %s :%d\n", strerror(errno), errno);
                            close(sockfd);
                            exit(EXIT_FAILURE);
                        }
                        if (sendto(*(client[1].sockfd), buf, 2, 0, client[1].client, client_len) < 0) {
                            fprintf(stderr, "Error in sending END message: %s :%d\n", strerror(errno), errno);
                            close(sockfd);
                            exit(EXIT_FAILURE);
                        }

                        #ifdef DEBUG_FUNC
                        printf("[DEBUG_FUNC] BUF message sent:%s\n", buf);
                        #endif

                        strncpy(board_state, "000000000", 9);   // reset board
                        moves = 0;                              // reset moves
                        i = 0;                                  // reset clients
                        game_finish = 1;                        // game finished

                        // TODO
                        // join thread to reject more servers
                        // if (pthread_join(threads[2],NULL) != 0) {
                        //     fprintf(stderr, "Error in joining Reject thread 0: %s :%d\n", strerror(errno), errno);
                        //     exit(EXIT_FAILURE);
                        // }

                        printf("Game ended, waiting for new connections\n");
                        game_finish = 0;                    // new game can start
                        break;                              // leave while loop
                    }   // if winner declared
                }   // while game is playing
            }   // if game_started
        }   // else new_client
    }   // main while loop to listen to clients

    free(buf);
    int j;
    for (j=0; j<i; j++) {
        free(client[j].recvd);
        free(client[j].client);
        free(client[j].msg);
        free(client[j].sockfd);
    }
    free(client);
    close(sockfd);
    return 0;
}


char check_win(){
    
    #ifdef DEBUG
    printf("[DEBUG] Entered check_win()\n");
    #endif

    int i;
    for (i=0;i<3;i++) {                                 // check for match in a row
        if (board_state[3*i] != '0')
            if (board_state[3*i] == board_state[3*i+1])
                if (board_state[3*i+1] == board_state[3*i+2])
                    return board_state[3*i];            // return player number
    }

    for (i=0;i<3;i++) {                                 // check for match in a col
        if (board_state[i] != '0')
            if (board_state[i] == board_state[3+i])
                if (board_state[3+i] == board_state[6+i])
                    return board_state[i];              // return player number
    }

    // check for match in first diagonal
    if (board_state[0] != '0')
        if (board_state[0] == board_state[4])
            if (board_state[4] == board_state[8])
                return board_state[0];                  // return player number

    // check for match in second diagonal
    if (board_state[2] != '0')
        if (board_state[2] == board_state[4])
            if (board_state[4] == board_state[6])
                return board_state[2];                  // return player number

    if (moves == 9) return '0';
    else return '3';
}


int update_move(char *buf, char player) {
    
    #ifdef DEBUG
    printf("[DEBUG] Entered update_move()\n");
    #endif

    int row = (int)(buf[1])-49;             // convert row char to int
    int col = (int)(buf[0])-49;             // convert col char to int

    #ifdef DEBUG_FUNC
    printf("[DEBUG_FUNC] row: %d col: %d\n", row, col);
    #endif

    if ((row>=0 && row<3) && (col>=0 && col<3))
        if (board_state[(3*row)+col] == '0') {  // check if position is empty
            board_state[(3*row)+col] = player;
            return 1;           // return success
        }
    return 0;              // return failure
}


void *welcomeThread(void *arg) {
    
    #ifdef DEBUG
    printf("[DEBUG] Entered welcomeThread()\n");
    #endif

    info_client *client = (info_client *) arg;
    socklen_t client_len = sizeof(*(client -> client));

    pthread_mutex_lock(&lock); // lock thread for reading data
    if (sendto(*(client->sockfd), client->msg, *(client->recvd), 0, client->client, client_len) < 0) {
        fprintf(stderr, "Error in sending confirmation message: %s :%d\n", strerror(errno), errno);
        close(*(client->sockfd));
        pthread_mutex_unlock(&lock); // unlock thread as data processing finished
        pthread_exit(NULL);
    }
    pthread_mutex_unlock(&lock); // unlock thread as data processing finished
    // printf("client: %s\n", client->client->sa_data);
    pthread_exit(NULL);
}

// TODO 
// This shit doesnt work

void *rejectThread (void *arg) {

    #ifdef DEBUG
    printf("[DEBUG] Entered rejectThread()\n");
    #endif

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Unable to create Reject Socket, %s - %d\n", strerror(errno), errno);
        pthread_exit(NULL);
    }

    int port = strtol(arg, NULL, 10);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    int binded = bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    if (binded < 0){
        fprintf(stderr, "Error in binding Reject Socket connection: %s :%d\n", strerror(errno), errno);
        close(sockfd);
        pthread_exit(NULL);
    }

    struct sockaddr client;
    socklen_t client_len;
    client_len = sizeof(client);

    // create END message
    char buf[2];
    buf[0] = 0x03; buf[1] = 0xFF;
    while (1) {

        if (game_finish) break;

        memset(buf, 0, sizeof(*buf));
        int recvd = recvfrom(sockfd, buf, LINE_SIZE, 0, &client, &client_len);
        if (recvd < 0) {
            fprintf(stderr, "Error in receiving Reject Thread data: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            pthread_exit(NULL);
        }

        memset(buf, 0, sizeof(*buf));
        buf[0] = 0x03; buf[1] = 0xFF;
        int sent = sendto(sockfd, buf, recvd, 0, &client, client_len);
        if (sent < 0) {
            fprintf(stderr, "Error in sending Rejection message: %s :%d\n", strerror(errno), errno);
            close(sockfd);
            pthread_exit(NULL);
        }
    
        #ifdef DEBUG
        printf("[DEBUG] Sent rejection message to a client\n");
        #endif

    }

    close(sockfd);
    // printf("client: %s\n", client->client->sa_data);
    pthread_exit(NULL);
}




        // // if client already exists then update message in its struct
        // int j, found = 0;
        // sockaddr_in *copy_client, *temp;
        // copy_client = (sockaddr_in *) &rcv_client;
        // for (j = 0; j<i; j++) {
        //     temp = (sockaddr_in *) client[j].client;
        //     if ( copy_client->sin_port == temp->sin_port && copy_client->sin_addr.s_addr == temp->sin_addr.s_addr ) {
        //         pthread_mutex_lock(&lock);
        //         memset(client[j].msg, 0, sizeof(*client[j].msg)); // reset previous message
        //         strncpy( client[j].msg, buf, recvd);
        //         *client[j].recvd = recvd;
        //         pthread_mutex_unlock(&lock);
        //         found = 1;
        //         break;
        //     }
        // }