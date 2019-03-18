#include "pthread08.h"

Thread *tptr;

int clifd[MAXNCLI], iget, iput;
pthread_mutex_t clifd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clifd_cond = PTHREAD_COND_INITIALIZER;
static int nthreads;

static void sig_int(int signo);

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 3) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 4) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: serv08 [ <host> ] <port#> <#threads>");
    }
    if (listenfd < 0) {
        err_sys("tcp_listen error");
    }
    nthreads = atoi(argv[argc - 1]);
    tptr = calloc(nthreads, sizeof(Thread));
    iget = iput = 0;

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

    struct sockaddr_storage cliaddr;
    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        int connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("accept error");
            }
        }

        pthread_mutex_lock(&clifd_mutex);
        clifd[iput] = connfd;
        if (++iput == MAXNCLI) {
            iput = 0;
        }
        if (iget == iput) {
            err_quit("iput = iget = %d", iput);
        }
        pthread_cond_signal(&clifd_cond);
        pthread_mutex_unlock(&clifd_mutex);
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
