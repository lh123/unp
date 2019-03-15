#include "ping.h"
#include <sys/time.h>

void send_v6(void) {
    struct icmp6_hdr *icmp6 = (struct icmp6_hdr *)sendbuf;
    icmp6->icmp6_type = ICMP6_ECHO_REQUEST;
    icmp6->icmp6_code = 0;
    icmp6->icmp6_id = htons(pid);
    icmp6->icmp6_seq = htons(++nsent);
    memset((icmp6 + 1), 0xa5, datalen);
    gettimeofday((struct timeval *)(icmp6 + 1), NULL);

    int len = 8 + datalen; // 8-byte ICMPv6 header

    if (sendto(sockfd, sendbuf, len, 0, pr->sasend, pr->salen) < 0) {
        err_sys("sendto error");
    }
}
