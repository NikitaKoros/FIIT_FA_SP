#ifndef TASK_1
#define TASK_1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
//#include <errno.h>

#include "errors.h"

#define _XOPEN_SOURCE 700
#define MAX_LOGIN_LENGTH 6
#define MAX_COMMAND_LENGTH 100
#define SANCTION_CONFIRMATION 12345
#define USERS_FILE "../users.data"

typedef struct {
    char login[MAX_LOGIN_LENGTH + 1];
    int pin;
    int max_requests;
    int current_requests;
} User;

typedef struct {
    Error error;
    union {
        time_t time_value;
        struct tm date_value;
        long long duration;
        User user;
        int integer;
        FILE* file;
    } data;
} Response;

typedef enum {
    SAME_LIMIT = -2,
    NO_LIMIT,
    BANNED
} Limit;

Error validate_login(const char*);
Response register_user(const char*, int);
Response authentificate_user(const char*, int);
Error update_user(const char*, int);
Response get_time();
Response get_date();
Response calculate_duration(const char*, char);
Response set_sanctions(const char*, int, int);
void command_loop(User*);

#endif