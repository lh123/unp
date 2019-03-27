#include "unp.h"
#include <sys/msg.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int msqid = msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT);
    if (msqid < 0) {
        err_sys("msgget error");
    }
    struct msgbuf buf;
    buf.mtype = 1;
    buf.mtext[0] = 1;

    if (msgsnd(msqid, &buf, 1, 0) < 0) {
        err_sys("msgsnd error");
    }
    struct msqid_ds info;
    if (msgctl(msqid, IPC_STAT, &info) < 0) {
        err_sys("msgctl error");
    }
    printf("read-write: %03o, cbytes = %lu, qnum = %lu, qbytes = %lu\n", 
            info.msg_perm.mode & 0777, (unsigned long)info.msg_cbytes, 
            (unsigned long)info.msg_qnum, (unsigned long)info.msg_qbytes);
    system("ipcs -q");
    if (msgctl(msqid, IPC_RMID, NULL) < 0) {
        err_sys("msgctl error");
    }
}
