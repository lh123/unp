#include "unp.h"
#include <fcntl.h>

static int servfd;
static int nsec;        // #secondadvio/udpserv04.outs between each alarm
static int maxnalarms;  // #alarmsadvio/udpserv04.out w/no client probe before quit
static int nprobes;     // #alarmsadvio/udpserv04.out since last client probe

static void sig_urg(int signo), sig_alrm(int signo);

void heartbeat_serv(int servfd_arg, int nsec_arg, int maxnalarms_arg) {
    servfd = servfd_arg;
    if ((nsec = nsec_arg) < 1) {
        nsec = 1;
    }
    if ((maxnalarms = maxnalarms_arg) < nsec) {
        maxnalarms = nsec;
    }

    struct sigaction act;
    act.sa_handler = sig_urg;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGURG, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    if (fcntl(servfd, F_SETOWN, getpid()) < 0) {
        err_sys("fcntl error");
    }

    act.sa_handler = sig_alrm;
    if (sigaction(SIGALRM, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    alarm(nsec);
}

static void sig_urg(int signo) {
    char c;
    ssize_t n = recv(servfd, &c, 1, MSG_OOB);
    if (n < 0) {
        if (errno != EWOULDBLOCK) {
            err_sys("recv error");
        }
    }
    if (send(servfd, &c, 1, MSG_OOB) < 0) {
        err_sys("send error");
    }
    nprobes = 0;
}

static void sig_alrm(int signo) {
    if (++nprobes > maxnalarms) {
        fprintf(stderr, "no probes from client\n");
        exit(0);
    }
    alarm(nsec);
}
