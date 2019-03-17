#ifndef _UDP_CKSUM_H
#define _UDP_CKSUM_H

#include "unp.h"
#include <pcap.h>

#include <netinet/ip.h>
#include <netinet/udp.h>

#define TTL_OUT 64 // outgoing TTL

extern struct sockaddr *dest, *local;
extern socklen_t destlen, locallen;
extern int datalink;
extern char *device;
extern pcap_t *pd;
extern int rawfd;
extern int snaplen;
extern int verbose;
extern int zerosum;

struct udpiphdr {
    struct ip ip;
    struct udphdr udp;
};

void cleanup(int signo);
char *next_pcap(int *len);
void open_output(void);
void open_pcap(void);
void send_dns_query(void);
void test_udp(void);
void udp_write(char *buf, int userlen);
struct udpiphdr *udp_read(void);

#endif // _UDP_CKSUM_H
