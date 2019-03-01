#include "unp.h"
#include <fcntl.h>

int connect_nonb(int sockfd, const SA *saptr, socklen_t salen, int nsec) {
    int flags = fcntl(sockfd, F_GETFL);
    if (flags < 0) {
        err_sys("fcntl error");
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        err_sys("fcntl error");
    }
    int n = connect(sockfd, saptr, salen);
    if (n < 0) {
        if (errno != EINPROGRESS) {
            return -1;
        }
    }
    if (n == 0) {
        goto done;
    }
    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_SET(sockfd, &rset);
    wset = rset;
    struct timeval tval;
    tval.tv_sec = nsec;
    tval.tv_usec = 0;

    if ((n = select(sockfd + 1, &rset, &wset, NULL, &tval)) < 0) {
        err_sys("select error");
    } else if (n == 0) {
        close(sockfd); // timeout
        errno = ETIMEDOUT;
        return -1;
    }
    int error;
    if (FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset)) {
        socklen_t len = sizeof(error);
        if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
            return -1;
        }
    } else {
        err_quit("select error: sockfd not set");
    }
done:
    if (fcntl(sockfd, F_SETFL, flags) < 0) {
        err_sys("fcntl error");
    }
    if (error) {
        close(sockfd);
        errno = error;
        return -1;
    }
    return 0;
}
