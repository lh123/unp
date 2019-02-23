#include "unp.h"
#include <fcntl.h>
#include <syslog.h>

// void debug(const char *msg) {
//     printf("%s: pid %d, ppid %d, pgid %d, sid %d\n", msg, (int)getpid(), (int)getppid(), (int)getpgid(0), (int)getsid(0));
// }

int daemon_init(const char *pname, int facility) {
    pid_t pid;
    int i;

    // debug("before first fork");
    if ((pid = fork()) < 0) {
        return -1;
    } else if (pid != 0) {
        exit(0);
    }

    // debug("before setsid");
    if (setsid() < 0) {
        return -1;
    }

    struct sigaction act;
    act.sa_handler = SIG_IGN;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGHUP, &act, NULL) < 0) {
        return -1;
    }

    // debug("before second fork");
    if ((pid = fork()) < 0) {
        return -1;
    } else if (pid != 0) {
        exit(0);
    }

    daemon_proc = 1;
    
    chdir("/");
    // debug("finish");

    for (i = 0; i < MAXFD; i++) {
        close(i);
    }

    open("/dev/null", O_RDONLY); // stdin
    open("/dev/null", O_RDWR); // stdout
    open("/dev/null", O_RDWR); // stderr

    openlog(pname, LOG_PID, facility);
    
    return 0;
}