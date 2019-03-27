#include "mesg.h"
#include "svmsg.h"
#include <sys/msg.h>

void sig_chld(int signo);
void server(int readid, int writeid);

int main(int argc, char **argv) {
    int msqid = msgget(MQ_KEY1, SVMSG_MODE | IPC_CREAT);
    if (msqid < 0) {
        err_sys("msgget error");
    }

    struct sigaction act;
    act.sa_handler = sig_chld;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGCHLD, &act, NULL) < 0) {
        err_sys("sigaction error");
    }
    
    server(msqid, msqid);
}
