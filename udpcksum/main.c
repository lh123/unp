#include "udpcksum.h"
#include <netdb.h>

struct sockaddr *dest, *local;
struct sockaddr_in locallookup;
socklen_t destlen, locallen;

int datalink;
char *device;
pcap_t *pd;
int rawfd;
int snaplen = 200;
int verbose;
int zerosum;

static void usage(const char *msg);

int main(int argc, char **argv) {
    opterr = 0;
    int c, lopt = 0;
    char *ptr, *localport, localname[1024];
    while ((c = getopt(argc, argv, "0i:l:v")) != -1) {
        switch (c) {
        case '0':
            zerosum = 1;
            break;
        case 'i':
            device = optarg;
            break;
        case 'l':
            if ((ptr = strchr(optarg, '.')) == NULL) {
                usage("invalid -l option");
            }
            *ptr++ = 0;
            localport = ptr;
            strncpy(localname, optarg, sizeof(localname));
            lopt = 1;
            break;
        case 'v':
            verbose = 1;
            break;
        case '?':
            usage("unrecognized option");
        }
    }
    if (optind != argc - 2) {
        usage("missing <host> and/or <serv>");
    }

    struct addrinfo *aip = host_serv(argv[optind], argv[optind + 1], AF_INET, SOCK_DGRAM);
    if (aip == NULL) {
        err_sys("host_serv error");
    }
    dest = calloc(1, aip->ai_addrlen);
    memcpy(dest, aip->ai_addr, aip->ai_addrlen);
    destlen = aip->ai_addrlen;
    freeaddrinfo(aip);
    if (verbose) {
        printf("dest addr %s\n", sock_ntop(dest, destlen));
    }

    if (lopt) {
        aip = host_serv(localname, localport, AF_INET, SOCK_DGRAM);
        if (aip == NULL) {
            err_sys("host_serv error");
        }
        local = calloc(1, aip->ai_addrlen);
        memcpy(local, aip->ai_addr, aip->ai_addrlen);
        locallen = aip->ai_addrlen;
        freeaddrinfo(aip);
    } else {
        int s = socket(AF_INET, SOCK_DGRAM, 0);
        if (connect(s, dest, destlen) < 0) {
            err_sys("connect error");
        }
        locallen = sizeof(locallookup);
        local = calloc(1, locallen);
        if (getsockname(s, local, &locallen) < 0) {
            err_sys("getsockname error");
        }
        if (((struct sockaddr_in *)local)->sin_addr.s_addr == htonl(INADDR_ANY)) {
            err_quit("Can't determine local address - use -l\n");
        }
        close(s);
    }
    if (verbose) {
        printf("local addr %s\n", sock_ntop(local, locallen));
    }

    open_output();

    open_pcap();

    setuid(getuid());

    struct sigaction act;
    act.sa_handler = cleanup;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGTERM, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    if (sigaction(SIGINT, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    if (sigaction(SIGHUP, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    test_udp();
    cleanup(0);
}

static void usage(const char *msg) {
    printf("%s\n", msg);
    printf("usage: udpcksum [ -i <dev> ] [ -0 ] [ -v ] <host> <serv>\n");
    exit(1);
}
