#include "mesg.h"
#include "svmsg.h"
#include <sys/msg.h>

void server(int readid, int writeid);

int main(int argc, char **argv) {
    int msqid = msgget(MQ_KEY1, SVMSG_MODE | IPC_CREAT);
    if (msqid < 0) {
        err_sys("msgget error");
    }
    server(msqid, msqid);
}
