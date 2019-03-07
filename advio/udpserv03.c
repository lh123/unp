#include "unp.h"
#include <net/if.h>

void myde_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen, struct sockaddr *myaddr);

int main(int argc, char **argv) {
    struct if_nameindex *ifi = if_nameindex();
    struct if_nameindex *ifptr;
    struct sockaddr_in cliaddr;
    int sockfd;
    const int on = 1;
    pid_t pid;
    for (ifptr = ifi; ifptr->if_name != NULL; ifptr++) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            err_sys("socket error");
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            err_sys("setsockopt error");
        }
        struct ifreq req;
        strncpy(req.ifr_name, ifptr->if_name, IFNAMSIZ);
        if (ioctl(sockfd, SIOCGIFADDR, &req) < 0) {
            err_sys("ioctl error");
        }
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        memcpy(&addr.sin_addr, &((struct sockaddr_in *)&req.ifr_addr)->sin_addr, sizeof(struct in_addr));
        addr.sin_port = htons(SERV_PORT);
        addr.sin_family = AF_INET;
        if (bind(sockfd, (SA *)&addr, sizeof(addr)) < 0) {
            err_sys("bind error");
        }
        printf("bind %s\n", sock_ntop((SA *)&addr, sizeof(addr)));
        if ((pid = fork()) < 0) {
            err_sys("fork error");
        } else if (pid == 0) {
            myde_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr), (SA *)&addr);
            exit(0);
        }
        if (ioctl(sockfd, SIOCGIFFLAGS, &req) < 0) {
            err_sys("ioctl error");
        }
        if (req.ifr_flags & IFF_BROADCAST) {
            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
                err_sys("setsockopt error");
            }
            if (ioctl(sockfd, SIOCGIFBRDADDR, &req) < 0) {
                err_sys("ioctl error");
            }
            memset(&addr, 0, sizeof(addr));
            memcpy(&addr.sin_addr, &((struct sockaddr_in *)&req.ifr_broadaddr)->sin_addr, sizeof(struct in_addr));
            addr.sin_port = htons(SERV_PORT);
            addr.sin_family = AF_INET;
            if (bind(sockfd, (SA *)&addr, sizeof(addr)) < 0) {
                if (errno = EADDRINUSE) {
                    printf("EADDRINUSE: %s\n", sock_ntop((SA *)&addr, sizeof(addr)));
                    close(sockfd);
                    continue;
                } else {
                    err_sys("bind error for %s", sock_ntop((SA *)&addr, sizeof(addr)));
                }
            }
            printf("bound %s\n", sock_ntop((SA *)&addr, sizeof(addr)));
            if ((pid = fork()) < 0) {
                err_sys("fork error");
            } else if (pid == 0) {
                myde_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr), (SA *)&addr);
                exit(0);
            }
        }
    }
    if_freenameindex(ifi);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    struct sockaddr_in wildaddr;
    memset(&wildaddr, 0, sizeof(wildaddr));
    wildaddr.sin_family = AF_INET;
    wildaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    wildaddr.sin_port = htons(SERV_PORT);
    if (bind(sockfd, (SA *)&wildaddr, sizeof(wildaddr)) < 0) {
        err_sys("bind error");
    }
    printf("bound %s\n", sock_ntop((SA *)&wildaddr, sizeof(wildaddr)));
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        myde_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr), (SA *)&wildaddr);
        exit(0);
    }
}

void myde_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen, struct sockaddr *myaddr) {
    char mesg[MAXLINE];
    for (;;) {
        socklen_t len = clilen;
        ssize_t n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
        printf("child %d, datagram from %s", (int)getpid(), sock_ntop(pcliaddr, clilen));
        printf(", to %s\n", sock_ntop(myaddr, clilen));
        if (sendto(sockfd, mesg, n, 0, pcliaddr, len) < 0) {
            err_sys("sendto error");
        }
    }
}
