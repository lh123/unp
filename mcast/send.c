#include "unp.h"
#include <sys/utsname.h>

#define SENDRATE 5

void send_all(int sendfd, SA *sadest, socklen_t salen) {
    struct utsname myname;
    if (uname(&myname) < 0) {
        err_sys("uname error");
    }
    char line[MAXLINE];
    ssize_t n = snprintf(line, sizeof(line), "%s, %d\n", myname.nodename, getpid());
    for (;;) {
        if (sendto(sendfd, line, n, 0, sadest, salen) < 0) {
            err_sys("sendto error");
        }
        sleep(SENDRATE);
    }
}
