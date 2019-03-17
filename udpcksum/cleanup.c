#include "udpcksum.h"

void cleanup(int signo) {
    putc('\n', stdout);
    if (verbose) {
        struct pcap_stat stat;
        if (pcap_stats(pd, &stat) < 0) {
            err_quit("pcap_stats: %s\n", pcap_geterr(pd));
        }
        printf("%d packets received by filter\n", stat.ps_recv);
        printf("%d packets dropper by kernel\n", stat.ps_drop);
    }
    exit(0);
}
