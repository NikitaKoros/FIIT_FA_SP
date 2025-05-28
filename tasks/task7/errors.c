#include "errors.h"

void LogErrors(Error err) {
    printf("Error occured: %s\n", ErrorToString(err));
}

const char *ErrorToString(const Error error) {
    switch(error) {
        case ERROR_NONE: return "ERROR_NONE";
        case ERROR_OPEN_DIR: return "ERROR_OPEN_DIR";
        case ERROR_STAT: return "ERROR_STAT";
        case ERROR_USER_GROUP: return "ERROR_USER_GROUP";
        default: return "UMKNOWN_ERROR";
    }
}