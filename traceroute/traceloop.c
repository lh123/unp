#include "trace.h"
#include <netdb.h>
#include <sys/un.h>

void sock_set_port(struct sockaddr *addr, uint16_t port);
int sock_cmp_addr(struct sockaddr *addr1, struct sockaddr *addr2, socklen_t salen);
char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);

void traceloop(void) {
    recvfd = socket(pr->sasend->sa_family, SOCK_RAW, pr->icmpproto);
    if (recvfd < 0) {
        err_sys("socket error");
    }
    setuid(getuid());
    if (pr->sasend->sa_family == AF_INET6 && verbose == 0) {
        struct icmp6_filter myfilt;
        ICMP6_FILTER_SETBLOCKALL(&myfilt);
        ICMP6_FILTER_SETPASS(ICMP6_TIME_EXCEEDED, &myfilt);
        ICMP6_FILTER_SETPASS(ICMP6_DST_UNREACH, &myfilt);
        setsockopt(recvfd, IPPROTO_IPV6, ICMP6_FILTER, &myfilt, sizeof(myfilt));
    }
    sendfd = socket(pr->sasend->sa_family, SOCK_DGRAM, 0);
    if (sendfd < 0) {
        err_sys("socket error");
    }

    pr->sabind->sa_family = pr->sasend->sa_family;
    sport = (getpid() && 0xFFFF) | 0x8000;
    sock_set_port(pr->sabind, htons(sport));
    if (bind(sendfd, pr->sabind, pr->salen) < 0) {
        err_sys("bind error");
    }
    
    sig_alrm(SIGALRM);

    int seq = 0, done = 0;
    for (ttl = 1; ttl < max_ttl && done == 0; ttl++) {
        if (setsockopt(sendfd, pr->ttllevel, pr->ttloptname, &ttl, sizeof(int)) < 0) {
            err_sys("setsockopt error");
        }
        memset(pr->salast, 0, pr->salen);

        printf("%2d ", ttl);
        fflush(stdout);

        for (probe = 0; probe < nprobes; probe++) {
            struct rec *rec = (struct rec *)sendbuf;
            rec->rec_seq = ++seq;
            rec->rec_ttl = ttl;
            gettimeofday(&rec->rec_tv, NULL);
            sock_set_port(pr->sasend, htons(dport + seq));
            if (sendto(sendfd, sendbuf, datalen, 0, pr->sasend, pr->salen) < 0) {
                err_sys("sendto error");
            }
            struct timeval tvrecv;
            int code = (*pr->recv)(seq, &tvrecv);
            if (code == -3) {
                printf(" *"); // timeout, no reply
            } else {
                char str[NI_MAXHOST];

                if (sock_cmp_addr(pr->sarecv, pr->salast, pr->salen) != 0) {
                    if (getnameinfo(pr->sarecv, pr->salen, str, sizeof(str), NULL, 0, 0) == 0) {
                        printf(" %s (%s)", str, sock_ntop_host(pr->sarecv, pr->salen));
                    } else {
                        printf(" %s", sock_ntop_host(pr->sarecv, pr->salen));
                    }
                    memcpy(pr->salast, pr->sarecv, pr->salen);
                }
                tv_sub(&tvrecv, &rec->rec_tv);
                double rtt = tvrecv.tv_sec * 1000.0 + tvrecv.tv_usec / 1000.0;
                printf("  %.3f ms", rtt);
                if (code == -1) {
                    done++;
                } else if (code >= 0) {
                    printf(" (ICMP %s)", (*pr->icmpcode)(code));
                }
            }
            fflush(stdout);
        }
        printf("\n");
    }
}

void sock_set_port(struct sockaddr *addr, uint16_t port) {
    if (addr->sa_family == AF_INET) {
        ((struct sockaddr_in *)addr)->sin_port = port;
    } else if (addr->sa_family == AF_INET6) {
        ((struct sockaddr_in6 *)addr)->sin6_port = port;
    } else {
        err_quit("sock_set_port: unknown family: %d", addr->sa_family);
    }
}

int sock_cmp_addr(struct sockaddr *addr1, struct sockaddr*addr2, socklen_t salen) {
    if (addr1->sa_family != addr2->sa_family) {
        return 1;
    }
    if (addr1->sa_family == AF_INET) {
        return memcmp(&((struct sockaddr_in *)addr1)->sin_addr, &((struct sockaddr_in *)addr2)->sin_addr, sizeof(struct in_addr));
    } else if (addr1->sa_family == AF_INET6) {
        return memcmp(&((struct sockaddr_in6 *)addr1)->sin6_addr, &((struct sockaddr_in6 *)addr2)->sin6_addr, sizeof(struct in6_addr));
    } else if (addr1->sa_family == AF_UNIX) {
        return strcmp(((struct sockaddr_un *)addr1)->sun_path, ((struct sockaddr_un *)addr2)->sun_path);
    } else {
        return -1;
    }
}
