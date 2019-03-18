#include "pthread08.h"

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
    printf("thread %d starting\n", i);

    for (;;) {
        pthread_mutex_lock(&clifd_mutex);
        while (iget == iput) {
            pthread_cond_wait(&clifd_cond, &clifd_mutex);
        }
        int connfd = clifd[iget];
        if (++iget == MAXNCLI) {
            iget = 0;
        }
        pthread_mutex_unlock(&clifd_mutex);
        tptr[i].thread_count++;

        web_child(connfd);
        close(connfd);
    }
}
