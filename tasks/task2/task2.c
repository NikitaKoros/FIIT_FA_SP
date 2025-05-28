// #include "util_task2.h"

//MAIN

// int main(int argc, char **argv) {
//     if (argc < 2) {
//         LogErrors(ERROR_INVALID_ARGUMENTS);
//         return 1;
//     }

//     int flag_index = -1;
//     for (int i = 1; i < argc; ++i) {
//         if (is_flag(argv[i])) {
//             flag_index = i;
//             break;
//         }
//     }

//     if (flag_index == -1 || flag_index == 1) {
//         LogErrors(ERROR_INVALID_FLAG);
//         return 1;
//     }

//     int num_files = flag_index - 1;
//     char **input_files = argv + 1;
//     char *flag = argv[flag_index];
//     char **flag_args = argv + flag_index + 1;
//     int num_flag_args = argc - flag_index - 1;

//     if (strncmp(flag, "xor", 3) == 0) {
//         int N = atoi(flag + 3);
//         if (N < 2 || N > 6) {
//             LogErrors(ERROR_INVALID_N);
//             return 1;
//         }

//         for (int i = 0; i < num_files; ++i) {
//             XorResponse resp = process_xor(input_files[i], N);
//             if (resp.error != ERROR_NONE) {
//                 LogErrors(resp.error);
//             } else {
//                 printf("%s: ", input_files[i]);
//                 for (size_t j = 0; j < resp.result_size; ++j)
//                     printf("%02X", resp.result[j]);
//                 printf("\n");
//                 free(resp.result);
//             }
//         }
//     } else if (strcmp(flag, "mask") == 0) {
//         if (num_flag_args < 1) {
//             LogErrors(ERROR_INVALID_ARGUMENTS);
//             return 1;
//         }

//         char *end;
//         unsigned long mask = strtoul(flag_args[0], &end, 16);
//         if (*end != '\0') {
//             LogErrors(ERROR_INVALID_MASK);
//             return 1;
//         }

//         for (int i = 0; i < num_files; ++i) {
//             MaskResponse resp = process_mask(input_files[i], mask);
//             if (resp.error != ERROR_NONE) {
//                 LogErrors(resp.error);
//             } else {
//                 printf("%s: %zu\n", input_files[i], resp.count);
//             }
//         }
//     } else if (strncmp(flag, "copy", 4) == 0) {
//         int N = atoi(flag + 4);
//         if (N <= 0) {
//             LogErrors(ERROR_INVALID_N);
//             return 1;
//         }

//         for (int i = 0; i < num_files; ++i) {
//             CopyResponse resp = process_copy(input_files[i], N);
//             if (resp.error != ERROR_NONE) {
//                 LogErrors(resp.error);
//             } else {
//                 printf("%s: %d copies created\n", input_files[i], N);
//             }
//         }
//     } else if (strcmp(flag, "find") == 0) {
//         if (num_flag_args < 1) {
//             LogErrors(ERROR_INVALID_ARGUMENTS);
//             return 1;
//         }

//         FindResponse resp = process_find(input_files, num_files, flag_args[0]);
//         if (resp.error != ERROR_NONE) {
//             LogErrors(resp.error);
//         } else if (resp.num_files == 0) {
//             printf("String not found in any file.\n");
//         } else {
//             printf("Found in files:\n");
//             for (size_t i = 0; i < resp.num_files; ++i) {
//                 printf("%s\n", resp.found_files[i]);
//                 free(resp.found_files[i]);
//             }
//             free(resp.found_files);
//         }
//     } else {
//         LogErrors(ERROR_INVALID_FLAG);
//         return 1;
//     }

//     return 0;
// }



//===========================================================================================================
//===========================================================================================================


// Test XOR

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <assert.h>
// #include "util_task2.h"

// void test_xor2() {
//     FILE *f = fopen("test_xor2.bin", "wb");
//     assert(f != NULL);
//     unsigned char data[] = {0xAF}; // 1010 1111
//     fwrite(data, 1, sizeof(data), f);
//     fclose(f);
    
//     XorResponse resp = process_xor("test_xor2.bin", 2);
//     assert(resp.error == ERROR_NONE);
//     assert(resp.result_size == 1);

//     // XOR: 1010 ^ 1111 = 0101 (0x5)
//     assert(resp.result[0] == 0x05);
    
//     free(resp.result);
//     remove("test_xor2.bin");
//     printf("test_xor2: PASSED\n");
// }

// void test_xor3() {
//     FILE *f = fopen("test_xor3.bin", "wb");
//     assert(f != NULL);
//     unsigned char data[] = {0x01, 0x02, 0x03, 0x04};
//     fwrite(data, 1, sizeof(data), f);
//     fclose(f);
    
//     XorResponse resp = process_xor("test_xor3.bin", 3);
//     assert(resp.error == ERROR_NONE);
//     assert(resp.result_size == 1);
    
//     // 0x01 ^ 0x02 ^ 0x03 ^ 0x04 = 0x04
//     assert(resp.result[0] == 0x04); 
    
//     free(resp.result);
//     remove("test_xor3.bin");
//     printf("test_xor3: PASSED\n");
// }

// void test_xor5() {
//     FILE *f = fopen("test_xor5.bin", "wb");
//     if (!f) {
//         printf("Failed to create test file\n");
//         return;
//     }
    
//     unsigned char block1[] = {0x01, 0x02, 0x03, 0x04};
//     unsigned char block2[] = {0x05, 0x06, 0x07, 0x08};
//     unsigned char block3[] = {0x09, 0x0A, 0x0B, 0x0C};
//     unsigned char block4[] = {0x0D, 0x0E, 0x0F, 0x10};
    
//     fwrite(block1, 1, sizeof(block1), f);
//     fwrite(block2, 1, sizeof(block2), f);
//     fwrite(block3, 1, sizeof(block3), f);
//     fwrite(block4, 1, sizeof(block4), f);
//     fclose(f);
    
//     // Run the XOR process
//     XorResponse resp = process_xor("test_xor5.bin", 5);
    
//     // Check for errors
//     assert(resp.error == ERROR_NONE);
//     assert(resp.result != NULL);
//     assert(resp.result_size == 4);
    
//     // For xor5, we should get the XOR of all 4-byte blocks
//     // Result should be block1 XOR block2 XOR block3 XOR block4
//     unsigned char expected[] = {
//         0x01 ^ 0x05 ^ 0x09 ^ 0x0D,
//         0x02 ^ 0x06 ^ 0x0A ^ 0x0E,
//         0x03 ^ 0x07 ^ 0x0B ^ 0x0F,
//         0x04 ^ 0x08 ^ 0x0C ^ 0x10
//     };
    
//     assert(memcmp(resp.result, expected, 4) == 0);
    
//     free(resp.result);
//     remove("test_xor5.bin");
//     printf("test_xor5: PASSED\n");
// }

// int main() {
//     printf("Testing XOR functions...\n");
//     test_xor2();
//     test_xor3();
//     test_xor5();
//     printf("All XOR tests passed!\n");
//     return 0;
// }

//===========================================================================================================
//===========================================================================================================

// TEST MASSK

// #include <stdio.h>
// #include <stdlib.h>
// #include <assert.h>
// #include "util_task2.h"

// void test_mask_basic() {
//     FILE *f = fopen("test_mask_basic.bin", "wb");
//     assert(f != NULL);
    
//     uint32_t data[] = {0x12345678, 0xAABBCCDD, 0x00FFFF00};
//     fwrite(data, sizeof(uint32_t), 3, f);
//     fclose(f);
    
//     MaskResponse resp = process_mask("test_mask_basic.bin", 0x00550000);
//     assert(resp.error == ERROR_NONE);
//     assert(resp.count == 1);
    
//     remove("test_mask_basic.bin");
//     printf("test_mask_basic: PASSED\n");
// }

// void test_mask_empty_file() {
//     FILE *f = fopen("test_mask_empty.bin", "wb");
//     assert(f != NULL);
//     fclose(f);
    
//     MaskResponse resp = process_mask("test_mask_empty.bin", 0xFF);
//     assert(resp.error == ERROR_NONE);
//     assert(resp.count == 0);
    
//     remove("test_mask_empty.bin");
//     printf("test_mask_empty: PASSED\n");
// }

// void test_mask_incomplete_bytes() {
//     FILE *f = fopen("test_mask_incomplete.bin", "wb");
//     assert(f != NULL);
    
//     uint32_t data[] = {0xAABBCCDD, 0x11223344};
//     uint8_t incomplete[] = {0x55, 0x66, 0x77};
    
//     fwrite(data, sizeof(uint32_t), 2, f);
//     fwrite(incomplete, 1, 3, f);
//     fclose(f);
    
//     MaskResponse resp = process_mask("test_mask_incomplete.bin", 0x00BB0000);
//     assert(resp.error == ERROR_NONE);
//     assert(resp.count == 1);
    
//     remove("test_mask_incomplete.bin");
//     printf("test_mask_incomplete: PASSED\n");
// }

// void test_mask_all_match() {
//     FILE *f = fopen("test_mask_all.bin", "wb");
//     assert(f != NULL);
    
//     uint32_t data[] = {0x123456FF, 0xABCDEFFF, 0x000000FF, 0x87654321};
//     fwrite(data, sizeof(uint32_t), 4, f);
//     fclose(f);
    
//     MaskResponse resp = process_mask("test_mask_all.bin", 0x000000FF);
//     assert(resp.error == ERROR_NONE);
//     assert(resp.count == 3);
    
//     remove("test_mask_all.bin");
//     printf("test_mask_all: PASSED\n");
// }

// void test_mask_no_match() {
//     FILE *f = fopen("test_mask_no_match.bin", "wb");
//     assert(f != NULL);
    
//     uint32_t data[] = {0x12345678, 0xAABBCCDD, 0xEEFF0011};
//     fwrite(data, sizeof(uint32_t), 3, f);
//     fclose(f);
    
//     MaskResponse resp = process_mask("test_mask_no_match.bin", 0x99999999);
//     assert(resp.error == ERROR_NONE);
//     assert(resp.count == 0);
    
//     remove("test_mask_no_match.bin");
//     printf("test_mask_no_match: PASSED\n");
// }

// void test_mask_file_error() {
//     MaskResponse resp = process_mask("nonexistent_file.bin", 0xFF);
//     assert(resp.error == ERROR_FILE_OPEN);
//     assert(resp.count == 0);
    
//     printf("test_mask_file_error: PASSED\n");
// }

// void test_mask_endianness() {
//     FILE *f = fopen("test_mask_endian.bin", "wb");
//     assert(f != NULL);
    
//     uint8_t data[] = {
//         0x78, 0x56, 0x34, 0x12,  // 0x12345678
//         0xDD, 0xCC, 0xBB, 0xAA   // 0xAABBCCDD
//     };
//     fwrite(data, 1, sizeof(data), f);
//     fclose(f);
    
//     MaskResponse resp = process_mask("test_mask_endian.bin", 0x12340000);
//     assert(resp.error == ERROR_NONE);
//     assert(resp.count == 1);
    
//     remove("test_mask_endian.bin");
//     printf("test_mask_endianness: PASSED\n");
// }

// int main() {
//     test_mask_basic();
//     test_mask_empty_file();
//     test_mask_incomplete_bytes();
//     test_mask_all_match();
//     test_mask_no_match();
//     test_mask_file_error();
//     test_mask_endianness();
    
//     printf("All tests PASSED\n");
//     return 0;
// }

//===========================================================================================================
//===========================================================================================================

// TEST COPY

// #include <stdio.h>
// #include <stdlib.h>
// #include <assert.h>
// #include <string.h>
// #include <stdint.h>
// #include <unistd.h>
// #include <linux/limits.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include "util_task2.h"

// int compare_file_contents(const char *file1, const char *file2) {
//     FILE *f1 = fopen(file1, "rb");
//     FILE *f2 = fopen(file2, "rb");
    
//     if (!f1 || !f2) {
//         if (f1) fclose(f1);
//         if (f2) fclose(f2);
//         return 0;
//     }
    
//     int result = 1;
//     uint8_t buffer1[4096], buffer2[4096];
//     size_t read1, read2;
    
//     while (1) {
//         read1 = fread(buffer1, 1, sizeof(buffer1), f1);
//         read2 = fread(buffer2, 1, sizeof(buffer2), f2);
        
//         if (read1 != read2 || memcmp(buffer1, buffer2, read1) != 0) {
//             result = 0;
//             break;
//         }
        
//         if (read1 < sizeof(buffer1)) {
//             break;
//         }
//     }
    
//     fclose(f1);
//     fclose(f2);
//     return result;
// }

// void test_copy_small_file() {
//     const char *filename = "test_copy_small.bin";
    
//     FILE *f = fopen(filename, "wb");
//     if (!f) {
//         printf("Failed to create test file\n");
//         return;
//     }
    
//     unsigned char data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
//     fwrite(data, 1, sizeof(data), f);
//     fclose(f);
    
//     int N = 3;
//     CopyResponse resp = process_copy(filename, N);
    
//     assert(resp.error == ERROR_NONE);
    
//     for (int i = 1; i <= N; i++) {
//         char copy_name[PATH_MAX];
//         snprintf(copy_name, PATH_MAX, "%s%d", filename, i);
       
//         FILE *check = fopen(copy_name, "rb");
//         assert(check != NULL);
//         fclose(check);
        
//         assert(compare_file_contents(filename, copy_name) == 1);
        
//         remove(copy_name);
//     }
    
//     remove(filename);
//     printf("test_copy_small_file: PASSED\n");
// }

// void test_copy_empty_file() {
//     const char *filename = "test_copy_empty.bin";
    
//     FILE *f = fopen(filename, "wb");
//     if (!f) {
//         printf("Failed to create test file\n");
//         return;
//     }
//     fclose(f);
    
//     int N = 2;
//     CopyResponse resp = process_copy(filename, N);
    
//     assert(resp.error == ERROR_NONE);
    
//     for (int i = 1; i <= N; i++) {
//         char copy_name[PATH_MAX];
//         snprintf(copy_name, PATH_MAX, "%s%d", filename, i);
        
//         FILE *check = fopen(copy_name, "rb");
//         assert(check != NULL);
//         fclose(check);
        
//         assert(compare_file_contents(filename, copy_name) == 1);
        
//         remove(copy_name);
//     }
    
//     remove(filename);
//     printf("test_copy_empty_file: PASSED\n");
// }

// void test_copy_larger_file() {
//     const char *filename = "test_copy_larger.bin";
    
//     FILE *f = fopen(filename, "wb");
//     if (!f) {
//         printf("Failed to create test file\n");
//         return;
//     }
    
//     for (int i = 0; i < 5000; i++) {
//         uint8_t data = (uint8_t)(i % 256);
//         fwrite(&data, 1, 1, f);
//     }
//     fclose(f);
    
//     int N = 5;
//     CopyResponse resp = process_copy(filename, N);
    
//     assert(resp.error == ERROR_NONE);
    
//     for (int i = 1; i <= N; i++) {
//         char copy_name[PATH_MAX];
//         snprintf(copy_name, PATH_MAX, "%s%d", filename, i);
        
//         FILE *check = fopen(copy_name, "rb");
//         assert(check != NULL);
//         fclose(check);
        
//         assert(compare_file_contents(filename, copy_name) == 1);
        
//         remove(copy_name);
//     }
    
//     remove(filename);
//     printf("test_copy_larger_file: PASSED\n");
// }

// void test_copy_nonexistent_file() {
//     const char *filename = "nonexistent_file.bin";

//     remove(filename);

//     int N = 2;
//     CopyResponse resp = process_copy(filename, N);

//     assert(resp.error == ERROR_FILE_WRITE);
    
//     printf("test_copy_nonexistent_file: PASSED\n");
// }

// int main() {
//     printf("Testing Copy functions...\n");
//     test_copy_small_file();
//     test_copy_empty_file();
//     test_copy_larger_file();
//     test_copy_nonexistent_file();
//     printf("All Copy tests passed!\n");
//     return 0;
// }

//===========================================================================================================
//===========================================================================================================

// TEST FIND

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "util_task2.h" 

void test_find_bytes_empty() {
    uint8_t buffer[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t search[] = {};
    
    int result = find_bytes(buffer, 4, search, 0);
    assert(result == 0);
    
    printf("test_find_bytes_empty: PASSED\n");
}

void test_find_bytes_found() {
    uint8_t buffer[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t search[] = {0x02, 0x03};
    
    int result = find_bytes(buffer, 5, search, 2);
    assert(result == 1);
    
    printf("test_find_bytes_found: PASSED\n");
}

void test_find_bytes_not_found() {
    uint8_t buffer[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t search[] = {0x02, 0x04};
    
    int result = find_bytes(buffer, 5, search, 2);
    assert(result == 0);
    
    printf("test_find_bytes_not_found: PASSED\n");
}

void test_find_bytes_at_beginning() {
    uint8_t buffer[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t search[] = {0x01, 0x02};
    
    int result = find_bytes(buffer, 5, search, 2);
    assert(result == 1);
    
    printf("test_find_bytes_at_beginning: PASSED\n");
}

void test_find_bytes_at_end() {
    uint8_t buffer[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t search[] = {0x04, 0x05};
    
    int result = find_bytes(buffer, 5, search, 2);
    assert(result == 1);
    
    printf("test_find_bytes_at_end: PASSED\n");
}

void test_process_find_single_file_found() {
    FILE *f = fopen("test_find1.txt", "w");
    assert(f != NULL);
    fprintf(f, "This is a test file with some content");
    fclose(f);
    
    char *filenames[] = {"test_find1.txt"};
    FindResponse resp = process_find(filenames, 1, "test");
    //LogErrors(resp.error);
    assert(resp.error == ERROR_NONE);
    
    assert(resp.num_files == 1);
    assert(resp.found_files != NULL);
    assert(strcmp(resp.found_files[0], "test_find1.txt") == 0);
    
    for (size_t i = 0; i < resp.num_files; i++) {
        free(resp.found_files[i]);
    }
    free(resp.found_files);
    remove("test_find1.txt");
    
    printf("test_process_find_single_file_found: PASSED\n");
}

void test_process_find_single_file_not_found() {
    FILE *f = fopen("test_find2.txt", "w");
    assert(f != NULL);
    fprintf(f, "This is a file with some content");
    fclose(f);
    
    char *filenames[] = {"test_find2.txt"};
    FindResponse resp = process_find(filenames, 1, "missing");
    
    assert(resp.error == ERROR_NONE);
    assert(resp.num_files == 0);
    assert(resp.found_files == NULL);
    
    remove("test_find2.txt");
    
    printf("test_process_find_single_file_not_found: PASSED\n");
}

void test_process_find_multiple_files() {
    FILE *f1 = fopen("test_find3a.txt", "w");
    assert(f1 != NULL);
    fprintf(f1, "This file has the search word");
    fclose(f1);
    
    FILE *f2 = fopen("test_find3b.txt", "w");
    assert(f2 != NULL);
    fprintf(f2, "This one doesn't have it");
    fclose(f2);
    
    FILE *f3 = fopen("test_find3c.txt", "w");
    assert(f3 != NULL);
    fprintf(f3, "This file also has search in it");
    fclose(f3);
    
    char *filenames[] = {"test_find3a.txt", "test_find3b.txt", "test_find3c.txt"};
    FindResponse resp = process_find(filenames, 3, "search");
    
    assert(resp.error == ERROR_NONE);
    assert(resp.num_files == 2);
    assert(resp.found_files != NULL);
    
    int found_a = 0, found_c = 0;
    for (size_t i = 0; i < resp.num_files; i++) {
        if (strcmp(resp.found_files[i], "test_find3a.txt") == 0) found_a = 1;
        if (strcmp(resp.found_files[i], "test_find3c.txt") == 0) found_c = 1;
    }
    
    assert(found_a == 1);
    assert(found_c == 1);
    
    for (size_t i = 0; i < resp.num_files; i++) {
        free(resp.found_files[i]);
    }
    free(resp.found_files);
    remove("test_find3a.txt");
    remove("test_find3b.txt");
    remove("test_find3c.txt");
    
    printf("test_process_find_multiple_files: PASSED\n");
}

void test_process_find_nonexistent_file() {
    char *filenames[] = {"nonexistent_file.txt"};
    FindResponse resp = process_find(filenames, 1, "test");
    
    assert(resp.error == ERROR_NONE);
    assert(resp.num_files == 0);
    
    printf("test_process_find_nonexistent_file: PASSED\n");
}

int main() {
    test_find_bytes_empty();
    test_find_bytes_found();
    test_find_bytes_not_found();
    test_find_bytes_at_beginning();
    test_find_bytes_at_end();
    
    test_process_find_single_file_found();
    test_process_find_single_file_not_found();
    test_process_find_multiple_files();
    test_process_find_nonexistent_file();
    
    printf("All tests PASSED\n");
    return 0;
}

//===========================================================================================================
//===========================================================================================================
