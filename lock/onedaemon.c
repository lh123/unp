#include "unp.h"
#include <fcntl.h>

#define PATH_PIDFILE    "pidfile"

int main(int argc, char **argv) {
    int pidfd;
    
    if ((pidfd = open(PATH_PIDFILE, O_RDWR | O_CREAT, FILEMODE)) < 0) {
        err_sys("open error");
    }
    if (write_lock(pidfd, 0, SEEK_SET, 0) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            err_quit("unable to lock %s, is %s already running?", PATH_PIDFILE, argv[0]);
        } else {
            err_sys("unable to lock %s", PATH_PIDFILE);
        }
    }
    char line[MAXLINE];
    snprintf(line, sizeof(line), "%ld\n", (long)getpid());
    if (ftruncate(pidfd, 0) < 0) {
        err_sys("ftruncate error");
    }
    if (write(pidfd, line, strlen(line)) < 0) {
        err_sys("write error");
    }
    pause();
}
