#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>

typedef enum {
    ERROR_NONE,
    ERROR_OPEN_DIR,
    ERROR_STAT,
    ERROR_USER_GROUP,
} Error;

void LogErrors(Error);
const char *ErrorToString(const Error);

#endif