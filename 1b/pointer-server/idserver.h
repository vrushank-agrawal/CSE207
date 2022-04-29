/*
 * idserver.h
 *
 *  Created on: Feb 15, 2016
 *      Author: jiaziyi
 *  Updated: JACF, 2020
 */

#ifndef IDSERVER_H_
#define IDSERVER_H_

struct idserver {
	char * id;
	char * region;	// eur asr ame afr
	int latency;	// in usec
	char status[8];	// up down unknown
	int *nthreads;
};

typedef struct idserver idserver;

void print_idserver(idserver s);

void modify(idserver s, char *id, int latency, char status[]);

void modify_by_pointer(idserver *s, char *id, int latency, char status[]);

idserver* create_idserver(char *id, char *region, int latency,
		char *status, int *nthreads);

#endif /* IDSERVER_H_ */
