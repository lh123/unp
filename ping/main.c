#include "ping.h"
#include <netdb.h>

struct proto proto_v4 = {&proc_v4, &send_v4, NULL, NULL, NULL, 0, IPPROTO_ICMP};
struct proto proto_v6 = {&proc_v6, &send_v6, init_v6, NULL, NULL, 0, IPPROTO_ICMPV6};

int datalen = 56;

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);

int main(int argc, char **argv) {
    opterr = 0;
    nsent = 0;
    int c;
    while ((c = getopt(argc, argv, "v")) != -1) {
        switch (c) {
        case 'v':
            verbose++;
            break;
        case '?':
        default:
            err_quit("unrecongnized option: -%c", optopt);
        }
    }
    if (optind != argc - 1) {
        err_quit("usage: ping [ -v ] <hostname>");
    }
    char *host = argv[optind];
    pid = getpid() & 0xFFFF; // ICMP ID field is 16 bits
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
    printf("PING %s (%s): %d data bytes\n", ai->ai_canonname ? ai->ai_canonname : h, h, datalen);
    if (ai->ai_family == AF_INET) {
        pr = &proto_v4;
    } else if (ai->ai_family == AF_INET6) {
        pr = &proto_v6;
        if (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr)) {
            err_quit("cannot ping IPv4-mapped IPv6 address");
        }
    } else {
        err_quit("unknown address family %d", ai->ai_family);
    }
    pr->sasend = ai->ai_addr;
    pr->sarecv = calloc(1, ai->ai_addrlen);
    pr->salen = ai->ai_addrlen;

    readloop();

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
