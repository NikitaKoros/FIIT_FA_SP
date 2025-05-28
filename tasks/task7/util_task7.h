#ifndef UTIL_TASK7_H
#define UTIL_TASK7_H

#include "errors.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

Error process_directory(char*);
void format_mode(mode_t, char*);
void format_time(time_t, char*);

#endif