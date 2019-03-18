#include "unp.h"
#include "pthread07.h"

Thread *tptr;

int listenfd, nthreads;
pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;

static void sig_int(int signo);

int main(int argc, char **argv) {
    if (argc == 3) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 4) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: serv07 [ <host> ] <port#> <#threads>");
    }
    nthreads = atoi(argv[argc - 1]);
    tptr = calloc(nthreads, sizeof(Thread));

    int i;
    for (i = 0; i < nthreads; i++) {
        thread_make(i);
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

static void sig_int(int signo) {
    void pr_cpu_time(void);

    pr_cpu_time();
    int i;
    for (i = 0; i < nthreads; i++) {
        printf("%ld ", tptr[i].thread_count);
    }
    printf("\n");
    exit(0);
}
