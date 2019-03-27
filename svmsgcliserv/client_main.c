#include "svmsg.h"

#include <fcntl.h>
#include <sys/msg.h>

void client(int readid, int writeid);

int main(int argc, char **argv) {
    int writeid = msgget(MQ_KEY1, 0);
    int readid = msgget(MQ_KEY2, 0);

    if (writeid < 0 || readid < 0) {
        err_sys("msgget error");
    }

    client(readid, writeid);

    if (msgctl(readid, IPC_RMID, NULL) < 0) {
        err_sys("msgctl error");
    }
    if (msgctl(writeid, IPC_RMID, NULL) < 0) {
        err_sys("msgctl error");
    }
}
