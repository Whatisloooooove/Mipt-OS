#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, const char* argv[]) {
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        close(fd);
        return -1;
    }
    char buffer[4096];
    ssize_t bytes_read = 0;

    while (1) {
        bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            close(fd);
            return -1;
        } else if (bytes_read == 0) {
            usleep(1);
            continue;
        }

        ssize_t bytes_write = write(STDOUT_FILENO, buffer, bytes_read);
        if (bytes_write < 0) {
                close(fd);
                return -1;
        }
    }
    close(fd);
    return 0;
}