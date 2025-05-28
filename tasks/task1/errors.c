#include "errors.h"

void LogErrors(Error err) {
    printf("Error occured: %s\n", ErrorToString(err));
}

const char *ErrorToString(const Error error) {
    switch(error) {
        case ERROR_NONE: return "ERROR_NONE";
        case ERROR_INVALID_LOGIN: return "ERROR_INVALID_LOGIN";
        case ERROR_INVALID_PIN: return "ERROR_INVALID_PIN";
        case ERROR_USER_ALREADY_EXISTS: return "ERROR_USER_EXISTS";
        case ERROR_USER_NOT_EXIST: return "ERROR_USER_NOT_EXIST";
        case ERROR_AUTH_FAILED: return "ERROR_AUTH_FAILED";
        case ERROR_INVALID_COMMAND: return "ERROR_INVALID_COMMAND";
        case ERROR_INVALID_TIME_FORMAT: return "ERROR_INVALID_TIME_FORMAT";
        case ERROR_INVALID_FLAG: return "ERROR_INVALID_FLAG";
        case ERROR_INVALID_REQUEST_LIMIT: return "ERROR_INVALID_REQUEST_LIMIT";
        case ERROR_SANCTION_CONFIRMATION_FAILED: return "ERROR_SANCTION_CONFIRMATION_FAILED";
        case ERROR_MEMORY_ALLOCATION: return "ERROR_MEMORY_ALLOCATION";
        case ERROR_USER_NOT_FOUND: return "ERROR_USER_NOT_FOUND";
        case ERROR_TOO_MANY_REQUESTS: return "ERROR_TOO_MANY_REQUESTS";
        case ERROR_INVALID_INPUT: return "ERROR_INVALID_INPUT";
        case ERROR_OPEN_FILE_FAILED: return "ERROR_OPEN_FILE_FAILED";
        case ERROR_WRITE_TO_FILE_FAILED: return "ERROR_WRITE_TO_FILE_FAILED";
        case ERROR_FAILED_TO_GET_TIME: return "ERROR_FAILED_TO_GET_TIME";
        default: return "UMKNOWN_ERROR";
    }
}