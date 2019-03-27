#include "svmsg.h"
#include <fcntl.h>
#include <sys/msg.h>

void client(int readfd, int writefd);

int main(int argc, char **argv) {
    int writeid = msgget(MQ_KEY1, 0);
    if (writeid < 0) {
        err_sys("msgget error");
    }
    int readid = msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT);
    if (readid < 0) {
        err_sys("msgget error");
    }
    client(readid, writeid);
    if (msgctl(readid, IPC_RMID, NULL) < 0) {
        err_sys("msgctl error");
    }
}
