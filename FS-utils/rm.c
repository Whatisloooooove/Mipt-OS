#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <getopt.h>

void remove_directory(const char *name) {
    struct dirent* dir_struct;
    DIR *dir = opendir(name);

    while ((dir_struct = readdir(dir))) {
        if (strcmp(dir_struct->d_name, ".") == 0 || strcmp(dir_struct->d_name, "..") == 0) {
            continue;
        }

        char* new_name = malloc(strlen(name) + strlen(dir_struct->d_name) + 2); //2 потому что для '/' и '/0' 
        sprintf(new_name, "%s/%s", name, dir_struct->d_name);
        new_name[strlen(name) + strlen(dir_struct->d_name) + 1] = '\0';

        struct stat file_inf;
        if (lstat(new_name, &file_inf) == -1) {
            free(new_name);
            continue;
        }

        if (S_ISDIR(file_inf.st_mode)) {
            remove_directory(new_name);
        } else {
            unlink(new_name);
        }

        free(new_name);
    }

    closedir(dir);

    rmdir(name);
}

int main(int argc, char *argv[]) {
    int recursive = 0;
    int opt = 0;
    while((opt = getopt(argc, argv, "r")) != -1) {
        if (opt == 'r') {
            recursive = 1;
        }
    }

    for (int ind = optind; ind < argc; ind++) {
        struct stat file_inf;
        if (lstat(argv[ind], &file_inf) == -1) {
            continue;
        }

        if (S_ISLNK(file_inf.st_mode)) {
            unlink(argv[ind]);
        } else if (S_ISDIR(file_inf.st_mode)) {
            if (recursive) {
                remove_directory(argv[ind]);
            } else {
                return -1;
            }
        } else {
            unlink(argv[ind]);
        }
    }

}
