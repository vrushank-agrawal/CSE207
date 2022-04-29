#include <stdio.h>
#include <string.h>
#include "fancy-hello-world.h"

int main(void) {
    int t = 10;
    char name[t], output[t+19];
    strcpy(output, "Hello world, hello ");
    fgets(name, t, stdin);                    // take input from user
    hello_string(name, output);                      // pass arrays to function
    return 0;
}

void hello_string(char* name, char* output) {
    strcat(output, name);                           // concatenate the arrays
    printf("%s\n", output);                           // print as a string
}