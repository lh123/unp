#include "sntp.h"
#include <net/if.h>
#include <time.h>

void sock_set_wild(SA *saddr);
void sntp_proc(char *buf, ssize_t n, struct timespec *nowptr);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: ssntp <IPaddress>");
    }
    struct sockaddr_storage mcastsa, wild, from;
    socklen_t salen = sizeof(mcastsa);
    int sockfd = udp_client(argv[1], "ntp", (SA *)&mcastsa, &salen);
    if (sockfd < 0) {
        err_sys("udp_client error");
    }
    memcpy(&wild, &mcastsa, salen); // copy family and port
    sock_set_wild((SA *)&wild);
    if (bind(sockfd, (SA *)&wild, salen) < 0) {
        err_sys("bind error");
    }
    struct if_nameindex* ifres = if_nameindex();
    struct if_nameindex* ifi;
    struct ifreq ireq;
    for (ifi = ifres; ifi->if_name != NULL; ifi++) {
        strncpy(ireq.ifr_name, ifi->if_name, IFNAMSIZ);
        if (ioctl(sockfd, SIOCGIFFLAGS, &ireq) < 0) {
            err_sys("ioctl error");
        }
        if (ireq.ifr_flags & IFF_MULTICAST) {
            if (mcast_join(sockfd, (SA *)&mcastsa, salen, NULL, ifi->if_index) < 0) {
                err_sys("mcast_join error");
            }
            printf("joined %s on %s\n", sock_ntop((SA *)&mcastsa, salen), ifi->if_name);
        }
    }
    if_freenameindex(ifres);
    
    char buf[MAXLINE];
    struct timespec now;
    for (;;) {
        socklen_t len = salen;
        ssize_t n = recvfrom(sockfd, buf, sizeof(buf), 0, (SA *)&from, &len);
        if (n < 0) {
            err_sys("recvfrom error");
        }
        clock_gettime(CLOCK_REALTIME, &now);
        sntp_proc(buf, n, &now);
    }
}

void sock_set_wild(SA *saddr) {
    switch (saddr->sa_family) {
    case AF_INET:
        ((struct sockaddr_in *)saddr)->sin_addr.s_addr = htonl(INADDR_ANY);
        break;
    case AF_INET6:
        memcpy(&((struct sockaddr_in6 *)saddr)->sin6_addr, &in6addr_any, sizeof(in6addr_any));
        break;
    default:
        errno = EAFNOSUPPORT;
    }
}
