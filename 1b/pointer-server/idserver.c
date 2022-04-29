/*
 * idserver.c
 *
 *  Created on: Feb 15, 2016
 *      Author: jiaziyi
 *  Updated: JACF, 2020
 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "idserver.h"

/**
 * print the server's information
 */
void print_idserver(idserver s)
{
	printf("Id: %s\n", s.id);
	printf("Latency (usec) : %d\n", s.latency);
	printf("Region: %s\n", s.region);
	printf("Status: %s\n", s.status);
	printf("Nbr of threads: %d\n", *s.nthreads);
}

/**
 * try to modify the server information
 */
void modify(idserver s, char *id, int latency, char status[])
{
	s.id = id;
	s.latency = latency;
	strncpy(s.status, status, sizeof(s.status));
}

/**
 * try to modify the student information using pointer
 */
void modify_by_pointer(idserver *s, char *id, int latency, char status[])
{	
	s->id = id;
	s->latency = latency;
	strncpy(s->status, status, sizeof(s->status));
}

idserver* create_idserver(char *id, char *region, int latency,
		char *status, int *nthreads)
{
	idserver *s;
	s = (idserver*)calloc(1, sizeof(idserver));
	if (s == NULL) {
		printf("Sorry, no memory left\n");
		return NULL;
	} // Got the inspiration on google to make my program more robust
	s->id = id;
	s->region = region;
	s->latency = latency;
	strncpy(s->status, status, sizeof(s->status));
	s->nthreads = nthreads;
	puts("---print inside create_idserver function---");
	print_idserver(*s);
	puts("---end of print inside");
	return s;
}
