#ifndef TASK2_H
#define TASK2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <linux/limits.h>
#include "errors.h"
#include <signal.h>

typedef struct {
    Error error;
    uint8_t *result;
    size_t result_size;
} XorResponse;

typedef struct {
    Error error;
    size_t count;
} MaskResponse;

typedef struct {
    Error error;
} CopyResponse;

typedef struct {
    Error error;
    char **found_files;
    size_t num_files;
} FindResponse;

int is_flag(const char *);
XorResponse process_xor(const char *, int);
MaskResponse process_mask(const char *, uint32_t);
CopyResponse process_copy(const char *, int);
int find_bytes(const uint8_t *, size_t, const uint8_t *, size_t);
FindResponse process_find(char **, int, const char *);

#endif