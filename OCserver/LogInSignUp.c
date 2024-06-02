#include <stdio.h>
#include <string.h>
#include "LogInSignUp.h"

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
    
    while (fscanf_s(fp, "%u|%19s|%19s", &line.user_id, line.username, (unsigned int)sizeof(line.username), line.password, (unsigned int)sizeof(line.password)) == 3) {
        printf("%u - %s - %s\n", line.user_id, line.username, line.password);
        if (!strcmp(username, line.username) && !strcmp(password, line.password)) {
            fclose(fp);
            return 1;
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

    while (fscanf_s(fp, "%u|%19s|%19s", &line.user_id, line.username, (unsigned int)(20 * sizeof(char)), line.password, (unsigned int)(20 * sizeof(char))) == 3) {
        printf("%u - %s - %s\n", line.user_id, line.username, line.password);
        // in_file меняется на 1. если пользователь имеется.
    }

    if (!in_file) {
        fputs(("%u|%s|%s", line.user_id + 1, username, password), fp);
        fputs("\n", fp);
        puts("good");
    }

    fclose(fp);


    //puts("reg");
    return 1;

}