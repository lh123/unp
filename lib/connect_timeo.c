#include "unp.h"

static void connect_alarm(int signo);

int connect_timeo(int sockfd, const struct sockaddr *saptr, socklen_t salen, int sec) {
    struct sigaction act, oact;
    act.sa_handler = connect_alarm;
    act.sa_flags = 0;
    sigfillset(&act.sa_mask);
    sigdelset(&act.sa_mask, SIGALRM);

    if (sigaction(SIGALRM, &act, &oact) < 0) {
        return -1;
    }

    if (alarm(sec) != 0) {
        err_msg("connect_timeo: alarm was already set");
    }

    int n = connect(sockfd, saptr, salen);
    if (n < 0) {
        close(sockfd);
        if (errno == EINTR) {
            errno = ETIMEDOUT;
        }
    }
    alarm(0);

    if (sigaction(SIGALRM, &oact, NULL) < 0) {
        err_msg("unable to restore previous signal handler");
    }

    return n;
}

static void connect_alarm(int signo) {
    // do nothing
}
