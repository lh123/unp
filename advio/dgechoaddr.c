#include "unp.h"
#include <net/if.h>

#undef MAXLINE
#define MAXLINE 20

ssize_t recvfrom_flags(int fd, void *ptr, size_t nbytes, int *flagsp, struct sockaddr *sa, socklen_t *salenptr, struct in_pktinfo *pktp);

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen) {
    struct in_pktinfo pktinfo;
    struct in_addr in_zero;
    memset(&in_zero, 0, sizeof(in_zero));
    
    char mesg[MAXLINE], str[INET6_ADDRSTRLEN], ifname[IFNAMSIZ];
    for (;;) {
        socklen_t len = clilen;
        int flags = 0;
        ssize_t n = recvfrom_flags(sockfd, mesg, MAXLINE, &flags, pcliaddr, &len, &pktinfo);
        if (n < 0) {
            err_sys("recvfrom_flags error");
        }
        printf("%d-byte datagram from %s", (int)n, sock_ntop(pcliaddr, len));
        if (memcmp(&pktinfo.ipi_addr, &in_zero, sizeof(in_zero)) != 0) {
            printf(", to %s", inet_ntop(AF_INET, &pktinfo.ipi_addr, str, sizeof(str)));
        }
        if (pktinfo.ipi_ifindex > 0) {
            printf(", recv i/f = %s", if_indextoname(pktinfo.ipi_ifindex, ifname));
        }
        if (flags & MSG_TRUNC) {
            printf(" (datagram truncated)");
        }
        if (flags & MSG_CTRUNC) {
            printf(" (control info truncated)");
        }
        printf("\n");
        if (sendto(sockfd, mesg, n, 0, pcliaddr, len) < 0) {
            err_sys("sendto error");
        }
    }
}
