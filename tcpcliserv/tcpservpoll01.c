#include "unp.h"
#include <limits.h>
#include <sys/poll.h>

int main(int argc, char **argv) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        err_sys("bind error");
    }

    
    if (listen(listenfd, LISTENQ) < 0) {
        err_sys("listen error");
    }
    int openmax;
    if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
        err_sys("sysconf(_SC_OPEN_MAX) error");
    }

    struct pollfd *client;
    if ((client = malloc(openmax * sizeof(struct pollfd))) == NULL) {
        err_sys("malloc error");
    }

    client[0].fd = listenfd;
    client[0].events = POLLRDNORM;

    int i, maxi;
    for (i = 1; i < openmax; i++) {
        client[i].fd = -1;
    }
    maxi = 0;

    for (;;) {
        int nready = poll(client, maxi + 1, -1);
        if (nready < 0) {
            err_sys("poll error");
        }
        struct sockaddr_in cliaddr;
        if (client[0].revents & POLLRDNORM) {
            socklen_t clilen = sizeof(cliaddr);
            int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
            if (connfd < 0) {
                err_sys("accept error");
            }
            for (i = 1; i < openmax; i++) {
                if (client[i].fd < 0) {
                    client[i].fd = connfd;
                    break;
                }
            }
            if (i == openmax) {
                err_quit("too many clients");
            }
            client[i].events = POLLRDNORM;
            if (i > maxi) {
                maxi = i;
            }
            if (--nready <= 0) {
                continue;
            }
        }

        for (i = 1; i <= maxi; i++) {
            int sockfd = client[i].fd;
            if (sockfd < 0) {
                continue;
            }
            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                char buf[MAXLINE];
                int n = read(sockfd, buf, MAXLINE);
                if (n < 0) {
                    if (errno == ECONNRESET) {
                        // connection reset by client
                        close(sockfd);
                        client[i].fd = -1;
                    } else {
                        err_sys("read error");
                    }
                } else if (n == 0) {
                    // connection closed by client
                    close(sockfd);
                    client[i].fd = -1;
                } else {
                    if (writen(sockfd, buf, n) < 0) {
                        err_sys("writen error");
                    }
                }
                if (--nready <= 0) {
                    break;
                }
            }
        }
    }
    free(client);
}
