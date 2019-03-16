#include "icmpd.h"

int readable_listen(void) {
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
    if (connfd < 0) {
        err_sys("accept error");
    }
    int i;
    for (i = 0; i < FD_SETSIZE; i++) {
        if (client[i].connfd < 0) {
            client[i].connfd = connfd;
            break;
        }
    }
    if (i == FD_SETSIZE) {
        close(connfd); // can't handle new client
        return --nready; // rudely close the new connection
    }
    printf("new connection, i = %d, connfd = %d\n", i, connfd);

    FD_SET(connfd, &allset);
    if (connfd > maxfd) {
        maxfd = connfd;
    }
    if (i > maxi) {
        maxi = i;
    }
    return --nready;
}
