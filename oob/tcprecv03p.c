#include "unp.h"
#include <sys/poll.h>

int main(int argc, char **argv) {
    int listenfd;
    if (argc == 2) {
        listenfd = tcp_listen(NULL, argv[1]);
    } else if (argc == 3) {
        listenfd = tcp_listen(argv[1], argv[2]);
    } else {
        err_quit("usage: tcprecv03p [host] <port>");
    }
    int connfd = accept(listenfd, NULL, NULL);
    struct pollfd pollfds[1];
    pollfds[0].fd = connfd;
    pollfds[0].events = POLLRDNORM;

    int justreadoob = 0;
    char buff[100];
    for (;;) {
        if (justreadoob == 0) {
            pollfds[0].events |= POLLRDBAND;
        }
        int n = poll(pollfds, 1, -1);
        if (n < 0 && errno == EINTR) {
            continue;
        } else if (n < 0) {
            err_sys("poll error");
        }

        if (pollfds[0].revents & POLLRDBAND) {
            ssize_t nr = recv(connfd, buff, sizeof(buff) - 1, MSG_OOB);
            if (nr < 0) {
                if (errno != EWOULDBLOCK && errno != EINTR) {
                    err_sys("recv error");
                }
            } else {
                buff[n] = 0;
                printf("read %d OOB byte: %s\n")
            }
        }
    }
}
