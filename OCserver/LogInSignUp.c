#include <stdio.h>
#include <string.h>
#include "LogInSignUp.h"

#include <ctype.h>

static char* client_db = "accountsDB.txt";

struct FileLine
{
    unsigned int user_id;
    char username[20];
    char password[20];
} line;

int log_in(char* username, char* password) {
    FILE* fp = fopen("clientsBD.txt", "r");

    if (fp == NULL) {
        perror("Error occured while opening clientsBD.txt");
        return 0;
    }

    fseek(fp, 0, SEEK_SET);

    char buffer[64]; // Буфер для строки
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {

        if (sscanf_s(buffer, "%3u|%[^|\n]|%[^|\n]", &line.user_id, line.username, (unsigned int)sizeof(line.username), line.password, (unsigned int)sizeof(line.password)) == 3) {

            if (!strcmp(username, line.username) && !strcmp(password, line.password)) {
                fclose(fp);
                return 1;
            }
        }

    }

    fclose(fp);
    return 0;
}

int sign_up(char* username, char* password) {
    FILE* fp = fopen("clientsBD.txt", "a+");
    int in_file = 0;
    line.user_id = 0;

    if (fp == NULL) {
        perror("Error occured while opening clientsBD.txt");
        return 0;
    }

    fseek(fp, 0, SEEK_SET);

    char buffer[64]; // Буфер для строки
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        
        if (sscanf_s(buffer, "%3u|%[^|\n]|%[^|\n]", &line.user_id, line.username, (unsigned int)sizeof(line.username), line.password, (unsigned int)sizeof(line.password)) == 3) {
           
            if (!strcmp(line.username, username)) {
                return 0;
            }
        }
        
    }
    
    if (!in_file) {
        fprintf(fp, "%3u|%s|%s\n", line.user_id + 1, username, password);
        puts("good");
    }

    fclose(fp);


    //puts("reg");
    return 1;

}