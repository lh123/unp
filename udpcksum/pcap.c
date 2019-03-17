#include "udpcksum.h"

#define CMD "udp and src host %s and src port %d"

char *sock_ntop_host(struct sockaddr *addr, socklen_t socklen);
int sock_get_port(SA *paddr);

void open_pcap(void) {
    char errbuf[PCAP_ERRBUF_SIZE];
    if (device == NULL) {
        if ((device = pcap_lookupdev(errbuf)) == NULL) {
            err_quit("pcap_lookup: %s", errbuf);
        }
    }
    printf("device = %s\n", device);

    if ((pd = pcap_open_live(device, snaplen, 0, 500, errbuf)) < 0) {
        err_quit("pcap_open_live: %s", errbuf);
    }
    uint32_t localnet, netmask;
    if (pcap_lookupnet(device, &localnet, &netmask, errbuf) < 0) {
        err_quit("pcap_lookupnet: %s", errbuf);
    }
    if (verbose) {
        char str1[INET_ADDRSTRLEN], str2[INET_ADDRSTRLEN];
        printf("localnet = %s, netmask = %s\n", inet_ntop(AF_INET, &localnet, str1, sizeof(str1)), inet_ntop(AF_INET, &netmask, str2, sizeof(str2)));
    }
    char cmd[MAXLINE];
    snprintf(cmd, sizeof(cmd), CMD, sock_ntop_host(dest, destlen), ntohs(sock_get_port(dest)));
    if (verbose) {
        printf("cmd = %s\n", cmd);
    }
    struct bpf_program fcode;
    if (pcap_compile(pd, &fcode, cmd, 0, netmask) < 0) {
        err_quit("pcap_compile: %s", pcap_geterr(pd));
    }
    if (pcap_setfilter(pd, &fcode) < 0) {
        err_quit("pcap_setfilter: %s", pcap_geterr(pd));
    }
    if ((datalink = pcap_datalink(pd)) < 0) {
        err_quit("pcap_datalink: %s", pcap_geterr(pd));
    }
    if (verbose) {
        printf("datalink = %d\n", datalink);
    }
}

char *next_pcap(int *len) {
    char *ptr;
    struct pcap_pkthdr hdr;
    while ((ptr = (char *)pcap_next(pd, &hdr)) == NULL)
        ;
    *len = hdr.caplen;
    return ptr;
}
