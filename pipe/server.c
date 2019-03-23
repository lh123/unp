#include "unp.h"
#include <fcntl.h>

void server(int readfd, int writefd) {
    char buf[MAXLINE];
    ssize_t n;

    if ((n = read(readfd, buf, MAXLINE)) < 0) {
        err_sys("read error");
    } else if (n == 0) {
        err_quit("end-of-file while reading pathname");
    }
    buf[n] = '\0';

    int fd = open(buf, O_RDONLY);
    if (fd < 0) {
        snprintf(buf + n, sizeof(buf) - n, ": can't open, %s\n", strerror(errno));
        n = strlen(buf);
        if (write(writefd, buf, n) < 0) {
            err_sys("write error");
        }
    } else {
        while ((n = read(fd, buf, MAXLINE)) > 0) {
            if (write(writefd, buf, n) < 0) {
                err_sys("write error");
            }
        }
        if (n < 0) {
            err_sys("read error");
        }
        close(fd);
    }
}
