#include "unp.h"
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(13);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }
    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }
    char buff[MAXLINE];
    for (;;) {
        int connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0) {
            err_sys("accept error");
        }
        time_t ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        if (write(connfd, buff, strlen(buff)) < 0) {
            err_sys("write error");
        }
        close(connfd);
    }
}