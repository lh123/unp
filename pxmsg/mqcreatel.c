#include "unp.h"
#include <fcntl.h>
#include <mqueue.h>

int main(int argc, char **argv) {
    int c;
    int flags = O_RDWR | O_CREAT;
    while ((c = getopt(argc, argv, "e")) != -1) {
        switch (c) {
        case 'e':
            flags |= O_EXCL;
            break;
        }
    }
    if (optind != argc - 1) {
        err_quit("usage: mqcreate [ -e ] <name>");
    }
    mqd_t mqd = mq_open(argv[optind], flags, FILEMODE, NULL);
    if (mqd < 0) {
        err_sys("mq_open error");
    }
    mq_close(mqd);
}
