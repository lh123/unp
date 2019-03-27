#include "unp.h"
#include <fcntl.h>
#include <sys/msg.h>

int main(int argc, char **argv) {
    int oflag = SVMSG_MODE | IPC_CREAT;
    int c;
    while ((c = getopt(argc, argv, "e")) != -1) {
        switch (c) {
        case 'e':
            oflag |= IPC_EXCL;
            break;
        case '?':
        default:
            err_quit("unknown option -%c", optopt);
        }
    }
    if (optind != argc - 1) {
        err_quit("usage: msgcreate [ -e ] <pathname>");
    }
    key_t key = ftok(argv[optind], 0);
    if (key < 0) {
        err_sys("ftok error");
    }
    int msqid = msgget(key, oflag);
    if (msqid < 0) {
        err_sys("msgget error");
    }
}
