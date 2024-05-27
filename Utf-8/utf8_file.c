#include "utf8_file.h"
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

utf8_file_t* utf8_fromfd(int fd) {
    utf8_file_t* utf8_file = (utf8_file_t*)malloc(sizeof(utf8_file_t));
    if (utf8_file) {
        utf8_file->fd = fd;
    }
    return utf8_file;
}

int count_bytes(uint8_t first_byte) {
        int expected_bytes = 0;
        if ((first_byte & 0x80) == 0) {
            expected_bytes = 1;
        } else if ((first_byte & 0xE0) == 0xC0) {
            expected_bytes = 2;
        } else if ((first_byte & 0xF0) == 0xE0) {
            expected_bytes = 3;
        } else if ((first_byte & 0xF8) == 0xF0) {
            expected_bytes = 4;
        } else if ((first_byte & 0xFC) == 0xF8) {
            expected_bytes = 5;
        } else if ((first_byte & 0xFE) == 0xFC) {
            expected_bytes = 6;
        } else {
            expected_bytes = -1;
        }
        return expected_bytes;
}

int utf8_read(utf8_file_t* f, uint32_t* res, size_t count) {
    int bytes_read = 0;
    while (count > 0) {
        uint8_t utf8_bytes[6];
        int expected_bytes = 0;
        uint8_t tmp[1];
        ssize_t bytes = read(f->fd, tmp, 1);
        if (bytes <= 0) {
            if (bytes_read > 0) {
                return bytes_read;
            } else if (bytes == 0) {
                return 0;
            } else {
                if (errno != 0) {
                    errno = ENOMEM;
                    return errno;
                }
                return -1;
            }
        }

        expected_bytes = count_bytes(tmp[0]);
        if (expected_bytes == -1) {
            return expected_bytes;
        }
        utf8_bytes[0] = tmp[0];
        for (int i = 1; i < expected_bytes; i++) {
            bytes = read(f->fd, utf8_bytes + i, 1);
            if (bytes <= 0 || (utf8_bytes[i] & 0xC0) != 0x80) {
                return -1;
            }
        }

        uint32_t code_point = utf8_bytes[0] & (0xFF >> (expected_bytes + 1));
        for (int i = 1; i < expected_bytes; i++) {
            code_point = (code_point << 6) | (utf8_bytes[i] & 0x3F);
        }
        res[bytes_read] = code_point;
        ++bytes_read;
        --count;
    }
    return bytes_read;
}

int utf8_write(utf8_file_t* f, const uint32_t* str, size_t count) {
    int bytes_writ = 0;
    for (; bytes_writ < count; bytes_writ++) {
        uint32_t code_point = str[bytes_writ];
        uint8_t utf8_bytes[4];
        int bytesToWrite = 0;


        if (code_point <= 0x7F) {
            utf8_bytes[0] = (uint8_t)code_point;
            bytesToWrite = 1;
        } else if (code_point <= 0x7FF) {
            utf8_bytes[0] = 0xC0 | ((code_point >> 6) & 0x1F);
            utf8_bytes[1] = 0x80 | (code_point & 0x3F);
            bytesToWrite = 2;
        } else if (code_point <= 0xFFFF) {
            utf8_bytes[0] = 0xE0 | ((code_point >> 12) & 0x0F);
            utf8_bytes[1] = 0x80 | ((code_point >> 6) & 0x3F);
            utf8_bytes[2] = 0x80 | (code_point & 0x3F);
            bytesToWrite = 3;
        } else if (code_point <= 0x10FFFF) {
            utf8_bytes[0] = 0xF0 | ((code_point >> 18) & 0x07);
            utf8_bytes[1] = 0x80 | ((code_point >> 12) & 0x3F);
            utf8_bytes[2] = 0x80 | ((code_point >> 6) & 0x3F);
            utf8_bytes[3] = 0x80 | (code_point & 0x3F);
            bytesToWrite = 4;
        } else {
            return -1;
        }

        ssize_t bytes = write(f->fd, utf8_bytes, bytesToWrite);
        if (bytes < 0) {
            return -1;
        } else if (bytes == 0) {
            return -1;
        }
        
    }
    return bytes_writ;
}