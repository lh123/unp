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
        printf("msqid = %d", msqid);
        if (msgctl(msqid, IPC_RMID, NULL) < 0) {
            err_sys("msgctl error");
        }
    }
}
