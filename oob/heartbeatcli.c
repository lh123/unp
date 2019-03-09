#include "unp.h"
#include <fcntl.h>

static int servfd;
static int nsec;        // #seconds between each alarm
static int maxnprobes;  // #probes w/no response before quit
static int nprobes;     // #probes since last server response

static void sig_urg(int signo), sig_alrm(int signo);

void heartbeat_cli(int servfd_arg, int nsec_arg, int maxnprobes_arg) {
    servfd = servfd_arg;
    if ((nsec = nsec_arg) < 1) {
        nsec = 1;
    }
    if ((maxnprobes = maxnprobes_arg) < nsec) {
        maxnprobes_arg = nsec;
    }
    nprobes = 0;

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
    nprobes = 0; // reset counter
}

static void sig_alrm(int signo) {
    if (++nprobes > maxnprobes) {
        fprintf(stderr, "server is unreachable\n");
        exit(0);
    }
    if (send(servfd, "1", 1, MSG_OOB) < 0) {
        err_sys("send error");
    }
    alarm(nsec);
}
