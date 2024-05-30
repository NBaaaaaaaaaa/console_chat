#include <stdio.h>

#include "LogInSignUp.h"

static char* client_db = "accountsDB.txt";

int log_in() {
	FILE* fp = fopen(client_db, 'r');

	if (fp == NULL) {
		puts("123");
	}

	fclose(fp);

}