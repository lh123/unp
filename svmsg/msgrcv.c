#include "unp.h"
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAXMSG (8192 + sizeof(long))

int main(int argc, char **argv) {
    long type = 0;
    int flag = 0, c;
    while ((c = getopt(argc, argv, "nt:")) != -1) {
        switch (c) {
        case 'n':
            flag |= IPC_NOWAIT;
            break;
        case 't':
            type = atol(optarg);
            break;
        case '?':
        default:
            err_quit("unknown option -%c", optopt);
        }
    }
    if (optind != argc - 1) {
        err_quit("usage: msgrcv [ -n ] [ -t type ] <pathname>");
    }

    key_t key = ftok(argv[optind], 0);
    if (key < 0) {
        err_sys("ftok error");
    }
    int mqid = msgget(key, S_IRUSR);
    if (mqid < 0) {
        err_sys("msgget error");
    }

    struct msgbuf *buf = malloc(MAXMSG);
    ssize_t n = msgrcv(mqid, buf, MAXMSG, type, flag);
    if (n < 0) {
        err_sys("msgrecv error");
    }
    printf("read %ld bytes, type = %ld\n", n, buf->mtype);
}
