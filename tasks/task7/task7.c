#include "util_task7.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        Error err = process_directory(".");
        if (err != ERROR_NONE) {
            LogErrors(err);
        }
    } else {
        Error err = process_directory(argv[1]);
        if (err != ERROR_NONE) {
            LogErrors(err);
        }
    }
    return 0;
}