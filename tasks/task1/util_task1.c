#include "util_task1.h"

Response open_user_db(const char* mode) {
    FILE* file = fopen(USERS_FILE, mode);
    if (!file) {
        return (Response){ERROR_OPEN_FILE_FAILED};
    }
    return (Response){ERROR_NONE, .data.file = file};
}

Error user_exists(const char* login) {
    Response res = open_user_db("rb");
    if (res.error != ERROR_NONE) {
        return res.error;
    }

    User user;

    FILE* db = res.data.file;
    while (fread(&user, sizeof(User), 1, db) == 1) {
        if (strcmp(user.login, login) == 0) {
            fclose(db);
            return ERROR_USER_ALREADY_EXISTS;
        }
    }

    fclose(db);
    return ERROR_NONE;
}

Error validate_login(const char* login) {
    if (strlen(login) > MAX_LOGIN_LENGTH) return ERROR_INVALID_LOGIN;
    for (size_t i = 0; i < strlen(login); i++) {
        if (!isalnum(login[i])) return ERROR_INVALID_LOGIN;
    }
    return ERROR_NONE;
}

Response register_user(const char* login, int pin) {
    Error err = validate_login(login);
    if (err != ERROR_NONE) {
        return (Response){err};
    }

    //printf("login to register:%s\n", login);
    err = user_exists(login);
    if (err != ERROR_NONE) {
        return (Response){err};
    }

    if (pin < 0 || pin > 100000) return (Response){ERROR_INVALID_PIN};

    Response res = open_user_db("ab");
    if (res.error != ERROR_NONE) {
        return res;
    }
    
    FILE* db = res.data.file;

    User new_user = {0};
    strncpy(new_user.login, login, MAX_LOGIN_LENGTH);
    new_user.pin = pin;
    new_user.max_requests = NO_LIMIT;

    if (fwrite(&new_user, sizeof(User), 1, db) != 1) {
        return (Response){ERROR_WRITE_TO_FILE_FAILED};
    }

    fclose(db);
    return (Response){ERROR_NONE};
}

Response authentificate_user(const char* login, int pin) {
    Response res = open_user_db("rb");
    if (res.error != ERROR_NONE) {
        return res;
    }

    User user;
    FILE* db = res.data.file;
    while (fread(&user, sizeof(User), 1, db) == 1) {
        if (strcmp(user.login, login) == 0 && user.pin == pin) {
            res.error = ERROR_NONE;
            res.data.user = user;
            break;
        }
    }
    
    fclose(db);
    return res;
}

Error update_user(const char* login, int new_max_requests) {
    Response res;
    //printf("%s\n", login);
    res = open_user_db("r+b");
    if (res.error != ERROR_NONE) {
        return res.error;
    }

    if (new_max_requests < SAME_LIMIT) {
        fclose(res.data.file);
        return ERROR_INVALID_REQUEST_LIMIT;
    }

    User old_user;
    long pos = 0;
    FILE* db = res.data.file;
    while (fread(&old_user, sizeof(User), 1, db) == 1) {
        //printf("found in file: %s\n", tmp.login);
        if (strcmp(login, old_user.login) == 0) {
            fseek(db, pos, SEEK_SET);
            User user = {0};
            if (new_max_requests == SAME_LIMIT) {
                strncpy(user.login, login, MAX_LOGIN_LENGTH);
                user.login[MAX_LOGIN_LENGTH] = '\0';
                user.pin = old_user.pin;
                user.max_requests = old_user.max_requests;
                user.current_requests++;
            } else {
                strncpy(user.login, login, MAX_LOGIN_LENGTH);
                user.login[MAX_LOGIN_LENGTH] = '\0';
                user.pin = old_user.pin;
                user.max_requests = new_max_requests;
                user.current_requests = old_user.current_requests;
            }
            if (fwrite(&user, sizeof(User), 1, db) != 1) {
                fclose(db);
                return ERROR_WRITE_TO_FILE_FAILED;
            }
            break;
        }
        pos = ftell(db);
    }

    fclose(db);
    return ERROR_NONE;
}

Response get_time() {
    time_t now = time(NULL);
    if (now == -1) {
        return (Response){ERROR_INVALID_TIME_FORMAT};
    } else {
        return (Response){ERROR_NONE, .data.time_value = now};
    }
}

Response get_date() {
    time_t now = time(NULL);
    if (now == -1) return (Response){ERROR_INVALID_TIME_FORMAT};
    struct tm* local = localtime(&now);
    if (!local) {
        return (Response){ERROR_FAILED_TO_GET_TIME};
    }
    Response res = (Response){ERROR_NONE, .data.date_value = *local};
    return res;
}

Response calculate_duration(const char* time_str, char flag) {
    struct tm tm = {0};
    // Формат ввода таргета: день:месяц:год часы:минуты:секунды
    if (!strptime(time_str, "%d:%m:%Y %H:%M:%S", &tm)){
        return (Response){ERROR_INVALID_TIME_FORMAT};
    }
    
    time_t target = mktime(&tm);
    if (target == -1) return (Response){ERROR_INVALID_TIME_FORMAT};
    
    double diff = difftime(time(NULL), target);
    long long res;
    switch (flag) {
        case 's': res = (long long)diff; break;
        case 'm': res = (long long)(diff / 60); break;
        case 'h': res = (long long)(diff / 3600); break;
        case 'y': res = (long long)(diff / (3600 * 24 * 365)); break;
        default: return (Response){ERROR_INVALID_FLAG};
    }
    return (Response){ERROR_NONE, .data.duration = res};
}

Response set_sanctions(const char* login, int limit, int confirm) {
    if (confirm != SANCTION_CONFIRMATION) {
        return (Response){ERROR_SANCTION_CONFIRMATION_FAILED};
    }
    
    Error err = user_exists(login);
    if (err != ERROR_USER_ALREADY_EXISTS) {
        return (Response){ERROR_USER_NOT_EXIST};
    }

    Response res = open_user_db("r+b");
    if (res.error != ERROR_NONE) {
        return res;
    }
    FILE* db = res.data.file;

    err = update_user(login, limit);
    if (err != ERROR_NONE) {
        fclose(db);
        return (Response){err};
    }
    
    fclose(db);
    return (Response){ERROR_NONE};
}

void command_loop(User* current_user) {
    char input[MAX_COMMAND_LENGTH];
    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            clearerr(stdin);
            continue;
        }
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "Logout") == 0) break;

        if (current_user->max_requests != -1 && 
            current_user->current_requests >= current_user->max_requests) {
            printf("Limit of %d requests per user reached!\n", current_user->max_requests);
            continue;
        }

        Response res;
        if (strcmp(input, "Time") == 0) {
            res = get_time();
            if (res.error == ERROR_NONE) {
                struct tm* tm = localtime(&res.data.time_value);
                printf("Current time: %02d:%02d:%02d\n", 
                       tm->tm_hour, tm->tm_min, tm->tm_sec);
                //free(tm);
                //free(res.data.time_value);
            }
        } else if (strcmp(input, "Date") == 0) {
            res = get_date();
            if (res.error == ERROR_NONE) {
                printf("Current date: %02d:%02d:%04d\n", 
                       res.data.date_value.tm_mday, 
                       res.data.date_value.tm_mon + 1, 
                       res.data.date_value.tm_year + 1900);
            }
        } else if (strncmp(input, "Howmuch", 7) == 0) {
            char date_time_str[40], flag[3];
            if (sscanf(input + 8, "%39[^-] %2s", date_time_str, flag) != 2) {
                res.error = ERROR_INVALID_INPUT;
            } else {
                res = calculate_duration(date_time_str, flag[1]);
            }
            if (res.error == ERROR_NONE) {
                printf("Time passed: %lld %c\n", res.data.duration, flag[1]);
            }
        } else if (strncmp(input, "Sanctions", 9) == 0) {
            char username[MAX_LOGIN_LENGTH + 1];
            int limit, confirm;
            if (sscanf(input + 9, "%6s %d %d", username, &limit, &confirm) != 3) {
                res.error = ERROR_INVALID_INPUT;
            } else {
                res = set_sanctions(username, limit, confirm);
            }
            if (res.error == ERROR_NONE) printf("Sanctions imposed\n");
        } else {
            res.error = ERROR_INVALID_COMMAND;
        }

        if (res.error != ERROR_NONE) {
            LogErrors(res.error);
        } else {
            update_user(current_user->login, SAME_LIMIT);
            current_user->current_requests++;
        }
    }
    fclose(stdin);
    fclose(stdout);
}
