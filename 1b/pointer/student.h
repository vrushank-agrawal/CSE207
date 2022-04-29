/*
 * pointer.h
 *
 *  Created on: Feb 15, 2016
 *      Author: jiaziyi
 */

#ifndef STUDENT_H_
#define STUDENT_H_

struct student {
	char * given_name;
	char * family_name;
	int age;
	char gender[8];
	int *promotion;
};

typedef struct student student;

void print_student(student s);

void modify(student s, char *given_name, int age, char *gender);

void modify_by_pointer(student *s, char *given_name, int age, char *gender);

student* create_student(char *given_name, char *family_name, int age,
		char *gender, int *promotion);

#endif /* STUDENT_H_ */
