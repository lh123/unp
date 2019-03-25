#include "unp.h"
#include <mqueue.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mqgetattr <name>");
    }
    mqd_t mqd = mq_open(argv[1], O_RDONLY);
    if (mqd < 0) {
        err_sys("mq_open error");
    }
    struct mq_attr attr;
    if (mq_getattr(mqd, &attr) < 0) {
        err_sys("mq_getattr error");
    }
    printf("max #msg = %ld, max #bytes/msg = %ld, #currently on queue = %ld\n", attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);
    mq_close(mqd);
}
