/**
 *  Jiazi Yi
 *
 * LIX, Ecole Polytechnique
 * jiazi.yi@polytechnique.edu
 *
 * Updated by Pierre Pfister
 *
 * Cisco Systems
 * ppfister@cisco.com
 *
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"url.h"

int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Missing argument. Please enter URL.\n");
		return 1; // main returning non-zero means error
	}

	char* url = argv[1];
	url_info info;

	int ret = parse_url(url, &info);
	if (ret) {
		fprintf(stderr, "Could not parse URL: %s\n", parse_url_errstr[ret]);
		return 2; // main returning non-zero means error
	}

	print_url_info(&info);
	return 0; // main returning 0 means success !
}


