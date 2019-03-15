#ifndef _PING_H
#define _PING_H

#include "unp.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#define BUFSIZE 1500

char sendbuf[BUFSIZE];

int datalen;
char *host;
int nsent;
pid_t pid;
int sockfd;
int verbose;

void init_v6(void);
void proc_v4(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv);
void proc_v6(char *ptr, ssize_t len, struct msghdr *msg, struct timeval *tvrecv);
void send_v4(void);
void send_v6(void);
void readloop(void);
void sig_alrm(int signo);
void tv_sub(struct timeval *out, struct timeval *in);
uint16_t in_cksum(uint16_t *addr, int len);

struct proto {
    void (*fproc)(char *, ssize_t, struct msghdr *, struct timeval *);
    void (*fsend)(void);
    void (*finit)(void);
    struct sockaddr *sasend;
    struct sockaddr *sarecv;
    socklen_t salen;
    int icmpproto;
} *pr;

#endif // _PING_H
