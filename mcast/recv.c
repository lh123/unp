#include "unp.h"

void recv_all(int recvfd, socklen_t salen) {
    struct sockaddr_storage safrom;
    char line[MAXLINE];
    for (;;) {
        socklen_t len = salen;
        ssize_t n = recvfrom(recvfd, line, MAXLINE, 0, (SA *)&safrom, &len);
        if (n < 0) {
            err_sys("recvfrom error");
        }
        line[n] = 0;
        printf("from %s: %s", sock_ntop((SA *)&safrom, len), line);
    }
}
