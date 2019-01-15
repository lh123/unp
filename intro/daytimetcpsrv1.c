#include "unp.h"
#include <time.h>

int main(int argc, char **argv) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(13);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }
    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }

    socklen_t len;
    struct sockaddr_in cliaddr;
    char buf[MAXLINE];
    for (;;) {
        len = sizeof(cliaddr);
        int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
        if (connfd < 0) {
            err_ret("accept error");
            continue;
        }
        printf("connection from %s, port %d\n", 
                inet_ntop(AF_INET, &cliaddr.sin_addr, buf, INET_ADDRSTRLEN), 
                ntohs(cliaddr.sin_port));
        time_t ticks = time(NULL);
        snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
        if (write(connfd, buf, strlen(buf)) <= 0) {
            err_sys("write error");
        }
        close(connfd);
    }
}
