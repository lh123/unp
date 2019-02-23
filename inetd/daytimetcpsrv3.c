#include "unp.h"
#include <time.h>
#include <syslog.h>

void daemon_inetd(const char *name, int facility);

int main(int argc, char **argv) {
    daemon_inetd(argv[0], 0);

    struct sockaddr_storage cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    if (getpeername(STDIN_FILENO, (struct sockaddr *)&cliaddr, &clilen) < 0) {
        err_sys("getpeername error");
    }
    err_msg("connection from %s", sock_ntop((struct sockaddr *)&cliaddr, clilen));

    char buff[MAXLINE];
    time_t ticks = time(NULL);
    snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
    
    if (writen(STDIN_FILENO, buff, strlen(buff)) < 0) {
        err_sys("write error");
    }
    close(STDIN_FILENO);
}

void daemon_inetd(const char *name, int facility) {
    daemon_proc = 1;
    openlog(name, LOG_PID, facility);
}
