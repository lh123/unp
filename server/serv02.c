#include "unp.h"

static int nchildren;
static pid_t *pids;
long *cptr;

long *meter(int nchildren);
pid_t child_make(int i, int listenfd);
void sig_int(int signo);

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 3) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 4) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: serv02 [ <host> ] <port#> <#children>");
    }
    nchildren = atoi(argv[argc - 1]);
    pids = calloc(nchildren, sizeof(pid_t));
    cptr = meter(nchildren);

    int i;
    for (i = 0; i < nchildren; i++) {
        pids[i] = child_make(i, listenfd);
    }

    struct sigaction act;
    act.sa_handler = sig_int;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGINT, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    for (;;) {
        pause();
    }
}

void sig_int(int signo) {
    void pr_cpu_time(void);

    int i;
    for (i = 0; i < nchildren; i++) {
        kill(pids[i], SIGTERM);
    }

    while (wait(NULL) > 0)
        ;
    if (errno != ECHILD) {
        err_sys("wait error");
    }
    pr_cpu_time();
    long total = 0;
    for (i = 0; i < nchildren; i++) {
        total += cptr[i];
        printf("%ld ", cptr[i]);
    }
    printf("%ld\n", total);
    exit(0);
}
