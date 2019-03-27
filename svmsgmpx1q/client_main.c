#include "svmsg.h"
#include <sys/msg.h>

void client(int readfd, int writefd);

int main(int argc, char **argv) {
    int msqid = msgget(MQ_KEY1, 0);
    if (msqid < 0) {
        err_sys("msgget error");
    }
    client(msqid, msqid);
}
