#pragma once

enum StatusClient
{
	NOT_AUTHORIZED,
	ENTER_USERNAME,
	ENTER_PASSWORD,
	AUTHORIZED
};

struct Client_un_pw
{
	int reg_or_log;		// reg = 0, log = 1;
	char username[20];
	char password[20];
};

int log_in(char* username, char* password);
int sign_up(char* username, char* password);