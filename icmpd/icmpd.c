#include "icmpd.h"
#include <sys/un.h>

int main(int argc, char **argv) {
    if (argc != 1) {
        err_quit("usage: icmpd");
    }
    maxi = -1;
    int i;
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i].connfd = -1;
    }
    FD_ZERO(&allset);
    if ((fd4 = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        err_sys("socket error");
    }
    FD_SET(fd4, &allset);
    if ((fd6 = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0) {
        err_sys("socket error");
    }
    FD_SET(fd6, &allset);
    maxfd = fd4 > fd6 ? fd4 : fd6;
    
    listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un sun;
    sun.sun_family = AF_LOCAL;
    strcpy(sun.sun_path, ICMPD_PATH);
    unlink(ICMPD_PATH);
    if (bind(listenfd, (SA *)&sun, sizeof(sun)) < 0) {
        err_sys("bind error");
    }
    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }
    FD_SET(listenfd, &allset);
    maxfd = maxfd > listenfd ? maxfd : listenfd;

    for (;;) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready < 0) {
            err_sys("select error");
        }
        if (FD_ISSET(listenfd, &rset)) {
            if (readable_listen() <= 0) {
                continue;
            }
        }
        if (FD_ISSET(fd4, &rset)) {
            if (readable_v4() <= 0) {
                continue;
            }
        }
        if (FD_ISSET(fd6, &rset)) {
            if (readable_v6() <= 0) {
                continue;
            }
        }
        for (i = 0; i <= maxi; i++) {
            int sockfd = client[i].connfd;
            if (sockfd < 0) {
                continue;
            }
            if (FD_ISSET(sockfd, &rset)) {
                if (readable_conn(i) <= 0) {
                    break;
                }
            }
        }
    }
}
