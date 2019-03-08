#include "unp.h"
#include <net/if.h>
#include <ifaddrs.h>

void myde_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen, struct sockaddr *myaddr);
in_port_t sock_get_port(const SA *addr, socklen_t len);
void sock_set_port(SA *addr, in_port_t port);
void sock_set_wild(SA *addr);

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    if (argc == 2) {
        sockfd = udp_client(NULL, argv[1], (SA *)&sa, &salen);
    } else if (argc == 3) {
        sockfd = udp_client(argv[1], argv[2], (SA *)&sa, &salen);
    } else {
        err_quit("usage: udpserv04 [ <host> ] <service or port>");
    }
    int family = sa.ss_family;
    in_port_t port = sock_get_port((SA *)&sa, salen);
    close(sockfd);

    struct sockaddr_storage cliaddr;
    struct ifaddrs *ifaddrlist, *ifptr;
    if (getifaddrs(&ifaddrlist) < 0) {
        err_sys("getifaddrs error");
    }
    const int on = 1;
    pid_t pid;
    for (ifptr = ifaddrlist; ifptr != NULL; ifptr = ifptr->ifa_next) {
        if (ifptr->ifa_addr == NULL || ifptr->ifa_addr->sa_family != family) {
            continue;
        }
        sockfd = socket(family, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            err_sys("socket error");
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            err_sys("setsockopt error");
        }
        sock_set_port(ifptr->ifa_addr, port);
        if (bind(sockfd, ifptr->ifa_addr, salen) < 0) {
            err_sys("bind error");
        }
        printf("bind %s\n", sock_ntop(ifptr->ifa_addr, salen));
        if ((pid = fork()) < 0) {
            err_sys("fork error");
        } else if (pid == 0) {
            myde_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr), ifptr->ifa_addr);
            exit(0);
        }
        if (ifptr->ifa_flags & IFF_BROADCAST && ifptr->ifa_broadaddr != NULL) {
            sockfd = socket(family, SOCK_DGRAM, 0);
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
                err_sys("setsockopt error");
            }
            sock_set_port(ifptr->ifa_broadaddr, port);
            if (bind(sockfd, ifptr->ifa_broadaddr, salen) < 0) {
                if (errno = EADDRINUSE) {
                    printf("EADDRINUSE: %s\n", sock_ntop(ifptr->ifa_broadaddr, salen));
                    close(sockfd);
                    continue;
                } else {
                    err_sys("bind error for %s", sock_ntop(ifptr->ifa_broadaddr, salen));
                }
            }
            printf("bound %s\n", sock_ntop(ifptr->ifa_broadaddr, salen));
            if ((pid = fork()) < 0) {
                err_sys("fork error");
            } else if (pid == 0) {
                myde_echo(sockfd, (SA *)&cliaddr, sizeof(cliaddr), ifptr->ifa_broadaddr);
                exit(0);
            }
        }
    }
    freeifaddrs(ifaddrlist);

    sockfd = socket(family, SOCK_DGRAM, 0);
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        err_sys("setsockopt error");
    }
    struct sockaddr_storage wildaddr;
    memcpy(&wildaddr, &sa, salen);
    sock_set_wild((SA *)&wildaddr);
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

in_port_t sock_get_port(const SA *addr, socklen_t len) {
    if (addr->sa_family == AF_INET) {
        return ntohs(((struct sockaddr_in *)addr)->sin_port);
    } else if (addr->sa_family == AF_INET6) {
        return ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
    } else {
        return -1;
    }
}

void sock_set_port(SA *addr, in_port_t port) {
    if (addr->sa_family == AF_INET) {
        ((struct sockaddr_in *)addr)->sin_port = htons(port);
    } else if (addr->sa_family == AF_INET6) {
        ((struct sockaddr_in6 *)addr)->sin6_port = htons(port);
    }
}

void sock_set_wild(SA *addr) {
    if (addr->sa_family == AF_INET) {
        struct in_addr zero_addr;
        zero_addr.s_addr = htonl(INADDR_ANY);
        memcpy(&((struct sockaddr_in *)addr)->sin_addr, &zero_addr, sizeof(zero_addr));
    } else if (addr->sa_family == AF_INET6) {
        memcpy(&((struct sockaddr_in6 *)addr)->sin6_addr, &in6addr_any, sizeof(in6addr_any));
    }
}
