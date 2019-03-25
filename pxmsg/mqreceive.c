#include "unp.h"
#include <mqueue.h>

int main(int argc, char **argv) {
    int c;
    int flags = O_RDONLY;
    while ((c = getopt(argc, argv, "n")) != -1) {
        switch (c) {
        case 'n':
            flags |= O_NONBLOCK;
            break;
        default:
            err_quit("unknown option -%c", optopt);
        }
    }
    if (optind != argc - 1) {
        err_quit("usage: mqreceive [ -n ] <name>");
    }
    mqd_t mqd = mq_open(argv[optind], flags);
    if (mqd < 0) {
        err_sys("mq_open error");
    }
    struct mq_attr attr;
    if (mq_getattr(mqd, &attr) < 0) {
        err_sys("mq_getattr error");
    }
    void *buff = malloc(attr.mq_msgsize);
    uint prio;
    ssize_t n = mq_receive(mqd, buff, attr.mq_msgsize, &prio);
    if (n < 0) {
        err_sys("mq_receive error");
    }
    printf("read %ld bytes, priority = %u\n", (long)n, prio);
    mq_close(mqd);
}
