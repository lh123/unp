#include "web.h"

void home_page(const char *host, const char *fname) {
    char line[MAXLINE];

    int fd = tcp_connect(host, SERV);
    int n = snprintf(line, sizeof(line), GET_CMD, fname);
    if (writen(fd, line, n) < 0) {
        err_sys("writen error");
    }
    for (;;) {
        if ((n = read(fd, line, MAXLINE)) == 0) {
            break;
        }
        printf("read %d bytes of home page\n", n);
        // do something
    }
    printf("end-of-file on home page\n");
    close(fd);
}
