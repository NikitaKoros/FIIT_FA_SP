#include "errors.h"
#include <stdio.h>

void LogErrors(Error err) {
    printf("Error occured: %s\n", ErrorToString(err));
}

const char *ErrorToString(const Error error) {
    switch(error) {
        case ERROR_NONE: return "ERROR_NONE";
        case ERROR_FILE_OPEN: return "ERROR_FILE_OPEN";
        case ERROR_FILE_READ: return "ERROR_FILE_READ";
        case ERROR_FILE_WRITE: return "ERROR_FILE_WRITE";
        case ERROR_INVALID_ARGUMENTS: return "ERROR_INVALID_ARGUMENTS";
        case ERROR_MEMORY_ALLOCATION: return "ERROR_MEMORY_ALLOCATION";
        case ERROR_INVALID_MASK: return "ERROR_INVALID_MASK";
        case ERROR_INVALID_FLAG: return "ERROR_INVALID_MASK";
        case ERROR_PROCESS_FAILURE: return "ERROR_INVALID_MASK";
        case ERROR_INVALID_N: return "ERROR_INVALID_MASK";
        default: return "UNKNOWN_ERROR";
    }
}

