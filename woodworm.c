#define _GNU_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/falloc.h>

void error(const char *reason)
{
    fprintf(stderr, "woodworm: %s: %s\n", reason, strerror(errno));
    exit(1);
}

int main(int argc, char **argv)
{
    const size_t block_size = 4 << 20;
    const ssize_t min_hole_size = 1 << 10;
    int n;
    for (n = 1; n < argc; n++) {
        const char *path = argv[n];
        off_t offset = 0;
        int fd;
        fd = open(path, O_RDWR);
        if (fd == -1)
            error(path);
        while (1) {
            char buffer[block_size];
            ssize_t len, l, r;
            len = read(fd, buffer, sizeof buffer);
            if (len == 0)
                break;
            if (len < 0)
                error(argv[n]);
            l = -1;
            for (r = 0; r < len; r++) {
                if (buffer[r] == '\0') {
                    if (l < 0)
                        l = r;
                } else {
                    if (l >= 0 && r - l >= min_hole_size) {
                        int rc = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset + l, r - l);
                        if (rc == -1)
                            error(path);
                    }
                    l = -1;
                }
            }
            if (l >= 0 && r - l >= min_hole_size) {
                int rc = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset + l, r - l);
                if (rc == -1)
                    error(path);
            }
            offset += len;
        }
        {
            int rc = close(fd);
            if (rc == -1)
                error(path);
        }
    }
    return 0;
}

/* vim:set ts=4 sw=4 et:*/
