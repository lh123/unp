#include "pthread07.h"

void *thread_main(void *arg);
void web_child(int fd);

void thread_make(int i) {
    int err = pthread_create(&tptr[i].thread_tid, NULL, &thread_main, (void *)(size_t)i);
    if (err != 0) {
        err_exit(err, "pthread_create error");
    }
}

void *thread_main(void *arg) {
    int i = (int)(size_t)arg;
    struct sockaddr_storage cliaddr;

    printf("thread %d starting\n", i);

    for (;;) {
        socklen_t clilen = sizeof(cliaddr);
        pthread_mutex_lock(&mlock);
        int connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
        pthread_mutex_unlock(&mlock);
        if (connfd < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                err_sys("accept error");
            }
        }
        tptr[i].thread_count++;
        web_child(connfd);
        close(connfd);
    }
}
