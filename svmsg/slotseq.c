#include "unp.h"
#include <fcntl.h>
#include <sys/msg.h>

int main(int argc, char **argv) {
    int i;
    for (i = 0; i < 10; i++) {
        int msqid = msgget(IPC_PRIVATE, 0644 | IPC_CREAT);
        if (msqid < 0) {
            err_sys("msgget error");
        }
        struct msqid_ds info;
        if (msgctl(msqid, IPC_STAT, &info) < 0) {
            err_sys("msgctl error");
        }
        printf("msqid = %d, seq = %d\n", msqid, info.msg_perm.__seq);
        if (msgctl(msqid, IPC_RMID, NULL) < 0) {
            err_sys("msgctl error");
        }
    }
}
