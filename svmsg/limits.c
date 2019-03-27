#include "unp.h"
#include <fcntl.h>
#include <sys/msg.h>

#define MAX_DATA    64 * 1024
#define MAX_NMESG   4096
#define MAX_NIDS    40000

int max_mesg;

struct mymesg {
    long type;
    char data[MAX_DATA];
} mesg;

int main(int argc, char **argv) {
    int msqid = msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT);
    if (msqid < 0) {
        err_sys("msgget error");
    }
    mesg.type = 1;
    
    int i, j;
    for (i = MAX_DATA; i > 0; i -= 128) {
        if (msgsnd(msqid, &mesg, i, 0) == 0) {
            printf("maximum amount of data per message = %d\n", i);
            max_mesg = i;
            break;
        }
        if (errno != EINVAL) {
            err_sys("msgsnd error for length %d", i);
        }
    }
    if (i == 0) {
        err_quit("i == 0");
    }
    if (msgctl(msqid, IPC_RMID, NULL) < 0) {
        err_sys("msgctl error");
    }
    mesg.type = 1;
    for (i = 8; i <= max_mesg; i *= 2) {
        if ((msqid = msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT)) < 0) {
            err_sys("msgget error");
        }
        for (j = 0; j < MAX_NMESG; j++) {
            if (msgsnd(msqid, &mesg, i, IPC_NOWAIT) != 0) {
                if (errno == EAGAIN) {
                    break;
                }
                err_sys("msgsnd error, i = %d, j = %d", i, j);
            }
        }
        printf("%d %d-byte messages were placed onto queue,", j, i);
        printf(" %d bytes total\n", i * j);
        if (msgctl(msqid, IPC_RMID, NULL) < 0) {
            err_sys("msgctl error");
        }
    }

    int qid[MAX_NIDS];
    mesg.type = 1;
    for (i = 0; i <= MAX_NIDS; i++) {
        if ((qid[i] = msgget(IPC_PRIVATE, SVMSG_MODE | IPC_CREAT)) == -1) {
            printf("%d identifiers open at once\n", i);
            break;
        }
    }
    for (j = 0; j < i; j++) {
        if (msgctl(qid[j], IPC_RMID, NULL) < 0) {
            err_sys("msgctl error");
        }
    }
}
