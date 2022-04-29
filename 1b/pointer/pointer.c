/*
 * pointer.c
 *
 *  Created on: Feb 15, 2016
 *      Author: jiaziyi
 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "student.h"

int main(int argc, char* argv[]){

	student bob, alice;
	student *s1, *s2;

	bob.age = 21;
	bob.family_name = "Smith";
	bob.given_name = "bob";
	strncpy(bob.gender, "male", strlen("male") + 1);
	int p = 2013;
	bob.promotion = &p;
	s1 = &bob;

	//step 1: pointer observation
	puts("=========step 1===========");
	printf("bob: %p\n", bob);
	printf("s1: %p\n", s1);
	printf("*s1: %p\n", *s1);
	printf("bob's name : %s\n", bob.given_name);
	printf("bob's age: %d \n", bob.age);
	printf("bob's promotion - first try: %d\n", bob.promotion);
	printf("bob's promotion - second try: %d\n", *bob.promotion);
	printf("bob's promotion through pointer - first try: %d\n", s1->promotion);
	printf("bob's promotion through pointer - second try: %d\n", *s1->promotion);
	puts("=========================");
	puts("");

	//step 2: print student
	puts("========step 2==========");
	puts("--results of print_student--");
	print_student(bob);
	puts("==========================");
	puts("");


//	//step 3: modification
//	puts("========step 3==========");
//	modify(bob, "bob", 24, "female");
//
//	puts("--results of modify--");
//	print_student(bob);
//
//	modify_by_pointer(&bob,"bob", 24, "female");
//	puts("--results of modify_by_pointer--");
//	print_student(bob);
//	puts("=======================");
//	puts("");
//
//	//step 4: pointers
//	puts("========step 4========");
//	alice = bob;
//	s2 = &alice;
//
//	modify_by_pointer(s2, "alice", 23, "female");
//	puts("--*s2--");
//	print_student(*s2);
//	puts("--alice--");
//	print_student(alice);
//	puts("--bob--");
//	print_student(bob);
//	puts("=====================");
//	puts("");
//
//
//	//step 5: create students
//	puts("========step 5=======");
//	int promotion = 2015;
//
//	student *s3 = create_student("ted", "green", 25, "female", &promotion);
//	puts("--results of creating ted, printed outside--");
//	print_student(*s3);
//	puts("=====================");
//	puts("");


	return EXIT_SUCCESS;
}


