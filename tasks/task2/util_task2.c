#include "util_task2.h"

int is_flag(const char *arg) {
    if (strncmp(arg, "xor", 3) == 0) {
        char *end;
        long n = strtol(arg + 3, &end, 10);
        if (*end == '\0' && n >= 2 && n <= 6) return 1;
    } else if (strcmp(arg, "mask") == 0) {
        return 1;
    } else if (strncmp(arg, "copy", 4) == 0) {
        char *end;
        long n = strtol(arg + 4, &end, 10);
        if (*end == '\0' && n > 0) return 1;
    } else if (strcmp(arg, "find") == 0) {
        return 1;
    }
    return 0;
}

XorResponse process_xor(const char *filename, int N) {
    XorResponse response = {ERROR_NONE, NULL, 0};

    const size_t bits_per_block = (1 << N); 
    const size_t bytes_per_block = (N >= 3) ? (bits_per_block / 8) : 0;
    const size_t bits_in_byte = 8;

    FILE *file = fopen(filename, "rb");
    if (!file) {
        response.error = ERROR_FILE_OPEN;
        return response;
    }

    size_t result_size = (N >= 3) ? bytes_per_block : 1;
    uint8_t *result = calloc(result_size, sizeof(uint8_t));
    if (!result) {
        fclose(file);
        response.error = ERROR_MEMORY_ALLOCATION;
        return response;
    }

    if (N >= 3) {
        uint8_t *block = malloc(bytes_per_block);
        if (!block) {
            free(result);
            fclose(file);
            response.error = ERROR_MEMORY_ALLOCATION;
            return response;
        }

        while (1) {
            size_t bytes_read = fread(block, 1, bytes_per_block, file);
            if (bytes_read == 0) {
                if (feof(file)) break;
                free(result);
                free(block);
                fclose(file);
                response.error = ERROR_FILE_READ;
                return response;
            }

            if (bytes_read < bytes_per_block) {
                memset(block + bytes_read, 0, bytes_per_block - bytes_read);
            }

            for (size_t i = 0; i < bytes_per_block; i++) {
                result[i] ^= block[i];
            }
        }

        free(block);

    } else {
        uint8_t current_byte;
        uint8_t bit_buffer = 0;
        size_t bit_counter = 0;

        while (fread(&current_byte, 1, 1, file) == 1) {
            for (int i = bits_in_byte - 1; i >= 0; i--) {
                uint8_t bit = (current_byte >> i) & 1;
                bit_buffer = (bit_buffer << 1) | bit;
                bit_counter++;

                if (bit_counter == bits_per_block) {
                    *result ^= bit_buffer;
                    bit_buffer = 0;
                    bit_counter = 0;
                }
            }
        }

        if (bit_counter > 0) {
            bit_buffer <<= (bits_per_block - bit_counter);
            result[0] ^= bit_buffer;
        }
    }

    fclose(file);
    response.result = result;
    response.result_size = result_size;
    return response;
}

MaskResponse process_mask(const char *filename, uint32_t mask) {
    MaskResponse response = {ERROR_NONE, 0};
    FILE *file = fopen(filename, "rb");
    if (!file) {
        response.error = ERROR_FILE_OPEN;
        return response;
    }

    uint8_t bytes[4];
    size_t bytes_read;
    while ((bytes_read = fread(bytes, 1, 4, file)) == 4) {
        uint32_t num = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
        if ((num & mask) == mask) {
            response.count++;
        }
    }

    if (ferror(file)) {
        response.error = ERROR_FILE_READ;
        fclose(file);
        return response;
    }

    fclose(file);
    return response;
}

CopyResponse process_copy(const char *filename, int N) {
    CopyResponse response = {ERROR_NONE};
    for (int i = 1; i <= N; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            response.error = ERROR_PROCESS_FAILURE;
            return response;
        } else if (pid == 0) {
            char new_name[PATH_MAX];
            snprintf(new_name, PATH_MAX, "%s%d", filename, i);

            FILE *src = fopen(filename, "rb");
            if (!src) {
                exit(1);
            }

            FILE *dst = fopen(new_name, "wb");
            if (!dst) {
                fclose(src);
                exit(1);
            }

            uint8_t buffer[BUFSIZ];   //
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                if (fwrite(buffer, 1, bytes_read, dst) != bytes_read) {
                    fclose(src);
                    fclose(dst);
                    exit(1);
                }
            }

            fclose(src);
            fclose(dst);
            exit(0);
        } else {
            int status;
            waitpid(pid, &status, 0);
            if WIFEXITED(status) {
                int exit_status = WEXITSTATUS(status);
                if (exit_status != 0)
                    response.error = ERROR_FILE_WRITE;
            } else {
                response.error = ERROR_PROCESS_FAILURE;
            }
        }
    }
    return response;
}

int find_bytes(const uint8_t *buffer, size_t buffer_len, const uint8_t *search, size_t search_len) {
    if (search_len == 0) return 0;
    for (size_t i = 0; i <= buffer_len - search_len; ++i) {
        if (memcmp(buffer + i, search, search_len) == 0) {
            return 1;
        }
    }
        
    return 0;
}

FindResponse process_find(char **filenames, int num_files, const char *search_str) {
    FindResponse response = {ERROR_NONE, NULL, 0};
    size_t search_len = strlen(search_str);
    uint8_t *search = (uint8_t*)search_str;

    pid_t *pids = malloc(num_files * sizeof(pid_t));
    if (!pids) {
        response.error = ERROR_MEMORY_ALLOCATION;
        //printf("1111");
        return response;
    }
    for (int i = 0; i < num_files; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            for (int j = 0; j < i; j++) {
                kill(pids[j], SIGTERM);
                waitpid(pids[j], NULL, 0);
            }
            free(pids);
            response.error = ERROR_PROCESS_FAILURE;
            return response;
        } else if (pid == 0) {
            FILE *file = fopen(filenames[i], "rb");
            if (!file) {
                exit(1);
            }

            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);

            uint8_t *buffer = malloc(size);
            if (!buffer) {
                fclose(file);
                exit(1);
            }

            size_t bytes_read = fread(buffer, 1, size, file);
            fclose(file);

            int found = 0;
            if (bytes_read == size) {
                found = find_bytes(buffer, bytes_read, search, search_len);
            }
            free(buffer);
            if (found) {
                exit(0);
            } else {
                exit(1);
            }
        } else {
            pids[i] = pid;
        }
    }

    char **found_files = NULL;
    size_t found_count = 0;

    for (int i = 0; i < num_files; ++i) {
        int status;
        pid_t pid = wait(&status);
        if (pid == -1) {
            response.error = ERROR_PROCESS_FAILURE;
            free(pids);
            
            for (size_t j = 0; i < found_count; j++) {
                free(found_files[j]);
            }
            free(found_files);
            return response;
        }

        int file_index = -1;
        for (int j = 0; j < num_files; j++) {
            if (pids[j] == pid) {
                file_index = j;
                break;
            }
        }
        if (file_index == -1) {
            response.error = ERROR_PROCESS_FAILURE;
            free(pids);
            for (size_t j = 0; j < found_count; j++) {
                free(found_files[j]);
            }
            free(found_files);
            return response;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            char** new_found_files = realloc(found_files, (found_count + 1) * sizeof(char*));
            if (!new_found_files) {
                response.error = ERROR_MEMORY_ALLOCATION;
                //printf("2222");

                free(pids);

                for (size_t j = 0; j < found_count; j++) {
                    free(found_files[j]);
                }
                free(found_files);
                return response;
            }
            found_files = new_found_files;

            found_files[found_count] = strdup(filenames[i]);
            if (!found_files[found_count]) {
                response.error = ERROR_MEMORY_ALLOCATION;
                //printf("3333");

                free(pids);

                for (size_t j = 0; j < found_count; j++) {
                    free(found_files[j]);
                }
                free(found_files);
                return response;
            }
            found_count++;
        }
    }

    free(pids);
    response.found_files = found_files;
    response.num_files = found_count;
    return response;
}

