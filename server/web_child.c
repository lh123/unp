#include "unp.h"

#define MAXN 16384

void web_child(int sockfd) {
    char line[MAXLINE], result[MAXN];

    for (;;) {
        ssize_t nread = readline(sockfd, line, MAXLINE);
        if (nread == 0) {
            return;
        }
        int ntowrite = atol(line);
        if (ntowrite <= 0 || ntowrite > MAXN) {
            err_quit("client request for %d bytes", ntowrite);
        }

        if (writen(sockfd, result, ntowrite) < 0) {
            err_sys("writen error");
        }
    }
}
