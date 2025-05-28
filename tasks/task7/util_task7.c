#include "util_task7.h"

Error process_directory(char* dirpath) {
    DIR* dir = opendir(dirpath);
    if (dir == NULL) {
        return ERROR_OPEN_DIR;
    }
    struct dirent* entry;
    Error err;
    while ((entry = readdir(dir)) != NULL) {
        struct stat st;
        char fullpath[PATH_MAX];
        snprintf(fullpath, PATH_MAX, "%s/%s", dirpath, entry->d_name);
        if (lstat(fullpath, &st) == -1) {
            LogErrors(ERROR_STAT);
            continue;
        }
        
        struct passwd* pwd = getpwuid(st.st_uid);
        struct group* grp = getgrgid(st.st_gid);
        if (!pwd || !grp) {
            LogErrors(ERROR_USER_GROUP);
            continue;
        }

        char mode_str[11];
        format_mode(st.st_mode, mode_str);

        char time_str[20];
        format_time(st.st_mtime, time_str);

        printf("%s %2lu %s %s %8ld %s %s\n\n",
            mode_str,
            (unsigned long)st.st_nlink,
            pwd->pw_name,
            grp->gr_name,
            (long)st.st_size,
            time_str,
            entry->d_name);
    }

    closedir(dir);
    return ERROR_NONE;
}

void format_mode(mode_t mode, char* str) {
    str[0] = (S_ISDIR(mode)) ? 'd' : (S_ISLNK(mode)) ? 'l' : '-';
    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

void format_time(time_t time, char* str) {
    struct tm* tm_info = localtime(&time);
    strftime(str, 20, "%b %e %H:%M", tm_info);
}