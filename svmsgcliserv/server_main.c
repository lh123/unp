#include "svmsg.h"

#include <fcntl.h>
#include <sys/msg.h>

void server(int readid, int writeid);

int main(int argc, char **argv) {
    int readid = msgget(MQ_KEY1, SVMSG_MODE | IPC_CREAT);
    int writeid = msgget(MQ_KEY2, SVMSG_MODE | IPC_CREAT);
    if (readid < 0 || writeid < 0) {
        err_sys("msgget error");
    }
    server(readid, writeid);
}
