#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/limits.h>

void create_parents(char* path) {
    char name[PATH_MAX];
    mode_t par_mode = 0775;
    for(int ind = 0; ind < strlen(path) ; ++ind) {
        name[ind] = path[ind];
        name[ind + 1] = '\0';
        if (path[ind + 1] == '/') {
            DIR* dir = opendir(name);
            if (dir == NULL) {
                mkdir(name, par_mode);
            } else {
                closedir(dir);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return -1;
    }
    int p_ind = 0;
    mode_t mode = 0775;
    int option_index = 0;
    int option = 0;

   static struct option long_options[] = {
        {"mode", required_argument, 0, 'm'},
        {"p", no_argument, NULL, 'p'},
        {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "m:p", long_options, &option_index)) != -1) {
        if (option == 'p') {
            p_ind = 1;
        }
        if (option == 'm') {
            mode = strtol(optarg, NULL, 8);
        }
    }

    for (int ind = optind; ind < argc; ++ind) { 
        char* path  = strdup(argv[ind]);
        if (p_ind) {
            create_parents(path);
        }
        return mkdir(argv[ind], mode);
        free(path);
    }
}