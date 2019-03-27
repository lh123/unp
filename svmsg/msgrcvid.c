#include "unp.h"
#include <sys/msg.h>

#define MAXMSG (8192 + sizeof(long))

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: msgrcvid <mqid>");
    }
    int mqid = atoi(argv[1]);
    struct msgbuf *buff = malloc(MAXMSG);
    if (buff == NULL) {
        err_sys("malloc error");
    }
    ssize_t n = msgrcv(mqid, buff, MAXMSG, 0, 0);
    if (n < 0) {
        err_sys("msgrcv error");
    }
    printf("read %ld bytes, type = %ld\n", n, buff->mtype);
}
