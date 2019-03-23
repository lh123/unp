#include "unp.h"

void client(int readfd, int writefd) {
    char buff[MAXLINE];
    if (fgets(buff, MAXLINE, stdin) == NULL) {
        err_sys("fgets error");
    }
    size_t len = strlen(buff);
    if (buff[len - 1] == '\n') {
        len--;
    }
    if (write(writefd, buff, len) < 0) {
        err_sys("write error");
    }

    ssize_t n;
    while ((n = read(readfd, buff, MAXLINE)) > 0) {
        if (write(STDOUT_FILENO, buff, n) < 0) {
            err_sys("write error");
        }
    }
    if (n < 0) {
        err_sys("read error");
    }
}
