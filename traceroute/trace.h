#ifndef _TRACE_H
#define _TRACE_H

#include "unp.h"
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#define BUFSIZE 1500

struct rec {
    uint16_t rec_seq;
    uint16_t rec_ttl;
    struct timeval rec_tv;
};

extern char recvbuf[BUFSIZE];
extern char sendbuf[BUFSIZE];

extern int datalen;
extern char *host;
extern uint16_t sport, dport;
extern int nsent;
extern pid_t pid;
extern int probe, nprobes;
extern int sendfd, recvfd;
extern int ttl, max_ttl;
extern int verbose;

const char *icmpcode_v4(int code);
const char *icmpcode_v6(int code);
int recv_v4(int seq, struct timeval *tv);
int recv_v6(int seq, struct timeval *tv);
void sig_alrm(int signo);
void traceloop(void);
void tv_sub(struct timeval *out, struct timeval *in);

struct proto {
    const char *(*icmpcode)(int);
    int (*recv)(int, struct timeval *);
    struct sockaddr *sasend;
    struct sockaddr *sarecv;
    struct sockaddr *salast;
    struct sockaddr *sabind;
    socklen_t salen;
    int icmpproto;
    int ttllevel;
    int ttloptname;
} *pr;

#endif // _TRACE_H
