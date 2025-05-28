#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    ERROR_NONE,
    ERROR_FILE_OPEN,
    ERROR_FILE_READ,
    ERROR_FILE_WRITE,
    ERROR_INVALID_ARGUMENTS,
    ERROR_MEMORY_ALLOCATION,
    ERROR_INVALID_MASK,
    ERROR_INVALID_FLAG,
    ERROR_PROCESS_FAILURE,
    ERROR_INVALID_N
} Error;

void LogErrors(Error);
const char *ErrorToString(const Error);

#endif