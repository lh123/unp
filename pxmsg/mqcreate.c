#include "unp.h"
#include <mqueue.h>

int main(int argc, char **argv) {
    int c;
    int flags = O_RDWR | O_CREAT;
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    while ((c = getopt(argc, argv, "em:z:")) != -1) {
        switch (c) {
        case 'e':
            flags |= O_EXCL;
            break;
        case 'm':
            attr.mq_maxmsg = atol(optarg);
            break;
        case 'z':
            attr.mq_msgsize = atol(optarg);
            break;
        case '?':
        default:
            err_quit("unknown option -%c", optopt);
            break;
        }
    }
    if (optind != argc - 1) {
        err_quit("usage: mqcreate [ -e ] [ -m <#maxmsg> -z <#msgsize> ] <name>");
    }
    if (((attr.mq_maxmsg != 0 && attr.mq_msgsize == 0) || (attr.mq_maxmsg == 0 && attr.mq_msgsize != 0))) {
        err_quit("must specify both -m <maxmsg> and -z <msgsize>");
    }
    mqd_t mqd = mq_open(argv[optind], flags, FILEMODE, (attr.mq_maxmsg != 0) ? &attr : NULL);
    if (mqd < 0) {
        err_sys("mq_open error");
    }
    mq_close(mqd);
}
