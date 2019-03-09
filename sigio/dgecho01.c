#include "unp.h"
#include <fcntl.h>

static int sockfd;

#define QSIZE 8     // size of input queue
#define MAXDG 4096  // max datagram size

typedef struct {
    char *dg_data[MAXDG];
    size_t dg_len;
    union {
        struct sockaddr_storage dg_data;
        struct sockaddr dg_sa[1];
    } dg_sau;
    socklen_t dg_salen;
} DG;

#define dg_sa dg_sau.dg_sa

static DG dg[QSIZE];            // queue of datagrams to process
static long cntread[QSIZE + 1]; // diagnostic counter
static int iget;                // next one for main loop to process
static int iput;                // next one for signal handler to read into
static int nqueue;              // # on queue for main loop to process
static socklen_t clilen;        // max length of sockaddr{}

static void sig_io(int signo);
static void sig_hup(int signo);

void dg_echo(int sockfd_arg, SA *pcliaddr, socklen_t clilen_arg) {
    sockfd = sockfd_arg;
    clilen = clilen_arg;
    int i;
    for (i = 0; i < QSIZE; i++) {
        dg[i].dg_salen = clilen;
    }
    iget = iput = nqueue = 0;

    struct sigaction act;
    act.sa_handler = sig_hup;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    if (sigaction(SIGHUP, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    act.sa_handler = sig_io;
    if (sigaction(SIGIO, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    if (fcntl(sockfd, F_SETOWN, getpid()) < 0) {
        err_sys("fcntl error");
    }
    int flags;
    if ((flags = fcntl(sockfd, F_GETFL)) < 0) {
        err_sys("fcntl error");
    }
    flags |= (O_ASYNC | O_NONBLOCK);
    if (fcntl(sockfd, F_SETFL, flags) < 0) {
        err_sys("fcntl error");
    }
    sigset_t zeromask, newmask, oldmask;
    sigemptyset(&zeromask);
    sigemptyset(&oldmask);
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGIO);

    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        err_sys("sigprocmask SIG_BLOCK error");
    }

    for (;;) {
        while (nqueue == 0) {
            sigsuspend(&zeromask); // wait for datagram to process
        }
        if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
            err_sys("sigprocmask SIG_SETMASK error");
        }
        if (sendto(sockfd, dg[iget].dg_data, dg[iget].dg_len, 0, dg[iget].dg_sa, dg[iget].dg_salen) < 0) {
            err_sys("sendto error");
        }
        if (++iget >= QSIZE) {
            iget = 0;
        }
        // block SIGIO
        if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
            err_sys("sigprocmask SIG_BLOCK error");
        }
        nqueue--;
    }
}

static void sig_io(int signo) {
    int nread = 0;
    for (;;) {
        if (nqueue >= QSIZE) {
            err_quit("receive overflow");
        }
        DG *ptr = &dg[iput];
        ptr->dg_salen = clilen;
        ssize_t len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0, ptr->dg_sa, &ptr->dg_salen);
        if (len < 0) {
            if (errno == EWOULDBLOCK) {
                break;
            } else {
                err_sys("recvfrom error");
            }
        }
        printf("recvfrom: %s\n", sock_ntop(ptr->dg_sa, ptr->dg_salen));
        ptr->dg_len = len;

        nread++;
        nqueue++;
        if (++iput >= QSIZE) {
            iput = 0;
        }
    }
    cntread[nread]++;
}

static void sig_hup(int signo) {
    int i;
    for (i = 0; i <= QSIZE; i++) {
        printf("cntread[%d] = %ld\n", i, cntread[i]);
    }
}
