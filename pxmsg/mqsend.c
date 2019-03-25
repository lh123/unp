#include "unp.h"
#include <mqueue.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        err_quit("usage: mqsend <name> <#bytes> <priority>");
    }
    size_t len = atol(argv[2]);
    uint prio = atoi(argv[3]);
    mqd_t mqd = mq_open(argv[1], O_WRONLY);
    if (mqd < 0) {
        err_sys("mq_open error");
    }
    void *ptr = calloc(len, sizeof(char));
    if (mq_send(mqd, ptr, len, prio) < 0) {
        err_sys("mq_send error");
    }
    mq_close(mqd);
}
