#include "trace.h"
#include <netdb.h>

struct proto proto_v4 = {icmpcode_v4, recv_v4, NULL, NULL, NULL, NULL, 0, IPPROTO_ICMP, IPPROTO_IP, IP_TTL};
struct proto proto_v6 = {icmpcode_v6, recv_v6, NULL, NULL, NULL, NULL, 0, IPPROTO_ICMPV6, IPPROTO_IPV6, IPV6_UNICAST_HOPS};

char recvbuf[BUFSIZE];
char sendbuf[BUFSIZE];

int datalen = sizeof(struct rec);
char *host;
uint16_t sport, dport = 32768 + 666;
int nsent;
pid_t pid;
int probe, nprobes = 3;
int sendfd, recvfd;
int ttl, max_ttl = 30;
int verbose;

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);

int main(int argc, char **argv) {
    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "m:v")) != -1) {
        switch (c) {
        case 'm':
            if ((max_ttl = atoi(optarg)) <= 1) {
                err_quit("invalid -m value");
            }
            break;
        case 'v':
            verbose++;
            break;
        case '?':
        default:
            err_quit("unrecognized option: -%c", optopt);
        }
    }
    if (optind != argc - 1) {
        err_quit("usage: traceroute [ -m <maxttl> -v ] <hostname>");
    }
    host = argv[optind];

    pid = getpid();
    struct sigaction act;
    act.sa_handler = sig_alrm;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGALRM, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    struct addrinfo *ai = host_serv(host, NULL, 0, 0);
    if (ai == NULL) {
        err_sys("host_serv error");
    }
    char *h = sock_ntop_host(ai->ai_addr, ai->ai_addrlen);
    printf("traceroute to %s (%s): %d hops max, %d data bytes\n", ai->ai_canonname ? ai->ai_canonname : h, h, max_ttl, datalen);

    if (ai->ai_family == AF_INET) {
        pr = &proto_v4;
    } else if (ai->ai_family == AF_INET6) {
        pr = &proto_v6;
        if (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr)) {
            err_quit("cannot traceroute IPv4-mapped IPv6 address");
        }
    } else {
        err_quit("unknown address family %d", ai->ai_family);
    }

    pr->sasend = ai->ai_addr;
    pr->sarecv = calloc(1, ai->ai_addrlen);
    pr->salast = calloc(1, ai->ai_addrlen);
    pr->sabind = calloc(1, ai->ai_addrlen);
    pr->salen = ai->ai_addrlen;

    traceloop();
}

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen) {
    static char ipstr[INET6_ADDRSTRLEN];
    if (addr->sa_family == AF_INET) {
        if (inet_ntop(AF_INET, &((struct sockaddr_in *)addr)->sin_addr, ipstr, sizeof(ipstr)) != NULL) {
            return ipstr;
        }
    } else if (addr->sa_family == AF_INET6) {
        if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *)addr)->sin6_addr, ipstr, sizeof(ipstr)) != NULL) {
            return ipstr;
        }
    }
    return NULL;
}
