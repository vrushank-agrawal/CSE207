/*
 * mutex.c
 *
 *  Created on: Mar 19, 2016
 *      Author: jiaziyi
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define NTHREADS 50

void *increase_counter(void *);

int counter = 0;

int main() {
	pthread_t threads[NTHREADS];
	int i, thread;
	//create 50 threads of increase_counter, each thread adding 1 to the counter
	for(i=0; i<NTHREADS; i++) {
		printf("%d\n",i);
		thread = pthread_create(&threads[i], NULL, increase_counter, &counter);
		if (thread) {
			fprintf(stderr, "Could not create thread: %s: %d\n", strerror(errno), errno);
			return -1;
		}
	}
	
	for(i=0; i<NTHREADS; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "Could not join thread: %s: %d\n", strerror(errno), errno);
			return -1;
		}
	}
	printf("\nFinal counter value: %d\n", counter);
	pthread_exit(NULL);
	return 0;
}

void *increase_counter(void *arg) {
	int i;
	// int counter = * (int *) arg;
	for(i=0;i<1000;i++) {
		// printf("%d\n",counter);
    	counter++;
	}
	pthread_exit(NULL);
}
