#include "web.h"
#include <netdb.h>
#include <fcntl.h>

void start_connect(struct file *fptr) {
    struct addrinfo *ai = host_serv(fptr->f_host, SERV, 0, SOCK_STREAM);
    if (ai == NULL) {
        err_quit("host_serv error for %s, %s", fptr->f_host, SERV);
    }
    int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    fptr->f_fd = fd;
    printf("start_connect for %s, fd %d\n", fptr->f_name, fptr->f_fd);

    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        err_sys("fcntl error");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        err_sys("fcntl error");
    }
    int n = connect(fd, ai->ai_addr, ai->ai_addrlen);
    if (n < 0) {
        if (errno != EINPROGRESS) {
            err_sys("nonblocking connect error");
        }
        fptr->f_flags = F_CONNECTING;
        FD_SET(fd, &rset);
        FD_SET(fd, &wset);
        if (fd > maxfd) {
            maxfd = fd;
        }
    } else if (n >= 0) {
        write_get_cmd(fptr);
    }
}
