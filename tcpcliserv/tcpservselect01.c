#include "unp.h"
#include <netinet/in.h>
#include <sys/select.h>

int main(int argc, char **argv) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err_sys("socket error");
    }
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }
    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }

    int maxfd = listenfd;
    int maxi = -1;
    int client[FD_SETSIZE], i;
    char buf[MAXLINE];
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }
    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    struct sockaddr_in a;
    for (;;) {
        rset = allset;
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            socklen_t clilen = sizeof(cliaddr);
            int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
            if (connfd < 0) {
                err_sys("accept error");
            }
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                err_quit("too many clients");
            }
            FD_SET(connfd, &allset);
            if (connfd > maxfd) {
                maxfd = connfd;
            }
            if (i > maxi) {
                maxi = i;
            }
            if (--nready <= 0) {
                continue;
            }
        }
        for (i = 0; i <= maxi; i++) {
            int sockfd = client[i];
            if (sockfd < 0) {
                continue;
            }
            if (FD_ISSET(sockfd, &rset)) {
                int n = read(sockfd, buf, MAXLINE);
                if (n < 0) {
                    err_sys("read error");
                } else if (n == 0) {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                    printf("client %d exit\n", sockfd);
                } else {
                    if (writen(sockfd, buf, n) < 0) {
                        err_sys("writen error");
                    }
                }
                if (--nready <= 0) {
                    break;
                }
            }
        }
    }
}
