# CSE207 Secret Assignment

This is the code for the secret assignment for the course CSE207.

## Instructions

- The server is created to host only 1 game but can be scaled to host more using the threads provided. 
- The game can be compiled using `make`.
- To run the server, user needs to run the file with the designated port as the input ex. `./server 4500`
- The client should then be run with the IP address and the server application's open port as variable arguments ex. `./client 127.0.0.1 4500`
- Any third server which tries to connect after 2 players are logged in is denied access.
- The game follows all the rules of a normal tic-tac-toe game with player 1 being assigned `x` and player 2 `o` for their moves. 
- The players should only input integer values between `0` and `2` (yeah we are in Computer Science) for the row and column on their turns.
- On input of a character value, the scanf function does not detect the wrong value type and this creates an infinite loop of inputs (simply put the user hacks the client)

## Test_server

- The test_server file contains a dummy server code provided to rigorously test our own code with.
- To run the test_server you may need to change execute settings using `chmod u=rwx`
- The exec.sh file is provided to test the test_server by creating 101 clients and check how many can be assigned.
- After several tests, it turns out that the test_server has bugs and is definitely not perfect.
