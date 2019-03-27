#include "unp.h"
#include <fcntl.h>
#include <sys/msg.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: msgrmid <pathname>");
    }
    key_t key = ftok(argv[optind], 0);
    if (key < 0) {
        err_sys("ftok error");
    }
    int mqid = msgget(key, 0);
    if (mqid < 0) {
        err_sys("msgget error");
    }
    if (msgctl(mqid, IPC_RMID, NULL) < 0) {
        err_sys("msgctl error");
    }
}
