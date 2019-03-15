#include "ping.h"
#include <sys/time.h>

void send_v4(void) {
    struct icmp *icmp = (struct icmp *)sendbuf;
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_id = htons(pid);
    icmp->icmp_seq = htons(++nsent);
    memset(icmp->icmp_data, 0xa5, datalen); // fill with pattern
    gettimeofday((struct timeval *)icmp->icmp_data, NULL);

    int len = 8 + datalen;
    
    icmp->icmp_cksum = 0;
    icmp->icmp_cksum = in_cksum((uint16_t *)icmp, len);
    if (sendto(sockfd, sendbuf, len, 0, pr->sasend, pr->salen) < 0) {
        err_sys("sendto error");
    }
}
