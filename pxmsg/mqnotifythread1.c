#include "unp.h"
#include <mqueue.h>
#include <pthread.h>

mqd_t mqd;
struct mq_attr attr;
struct sigevent sigev;

static void notify_thread(union sigval arg);

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mqnotifythread1 <name>");
    }
    if ((mqd = mq_open(argv[1], O_RDONLY | O_NONBLOCK)) < 0) {
        err_sys("mq_open error");
    }
    if (mq_getattr(mqd, &attr) < 0) {
        err_sys("mq_getattr error");
    }
    sigev.sigev_notify = SIGEV_THREAD;
    sigev.sigev_value.sival_ptr = NULL;
    sigev.sigev_notify_function = notify_thread;
    sigev.sigev_notify_attributes = NULL;
    if (mq_notify(mqd, &sigev) < 0) {
        err_sys("mq_notify error");
    }

    for (;;) {
        pause();
    }
}

static void notify_thread(union sigval arg) {
    printf("notify_thread started\n");
    void *buff = malloc(attr.mq_msgsize);
    if (buff == NULL) {
        err_sys("malloc error");
    }
    if (mq_notify(mqd, &sigev) < 0) {
        err_sys("mq_notify error");
    }

    ssize_t n;
    while ((n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) {
        printf("read %ld bytes\n", (long)n);
    }
    if (errno != EAGAIN) {
        err_sys("mq_receive error");
    }
    free(buff);
    pthread_exit(NULL);
}
