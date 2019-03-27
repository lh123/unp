#include "unp.h"
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/ipc.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        err_quit("usage: msgsnd <pathname> <#bytes> <type>");
    }
    size_t len = atol(argv[2]);
    long type = atol(argv[3]);
    key_t key = ftok(argv[optind], 0);
    if (key < 0) {
        err_sys("ftok error");
    }
    int mqid = msgget(key, S_IWUSR);
    struct msgbuf *ptr = calloc(sizeof(long) + len, sizeof(char));
    ptr->mtype = type;
    if (msgsnd(mqid, ptr, len, 0) < 0) {
        err_sys("msgsnd error");
    }
}
