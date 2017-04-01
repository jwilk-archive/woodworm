/* Copyright © 2013 Jakub Wilk <jwilk@jwilk.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

/* vim:set ts=4 sts=4 sw=4 et:*/
