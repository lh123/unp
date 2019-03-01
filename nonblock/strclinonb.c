#include "unp.h"
#include <netdb.h>
#include <fcntl.h>
#include <time.h>

#define DECLARE_MAX(type) \
    type max_##type(type a, type b) { \
        return a > b ? a : b; \
    }

DECLARE_MAX(int)

void str_cli(FILE *fp, int sockfd);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: tcpcli <hostname/IPaddress> <service/port#>");
    }
    struct addrinfo hint, *res, *addrptr;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(argv[1], argv[2], &hint, &res) < 0) {
        err_sys("getaddrinfo error");
    }
    int sockfd;
    for (addrptr = res; addrptr != NULL; addrptr = addrptr->ai_next) {
        if ((sockfd = socket(addrptr->ai_family, addrptr->ai_socktype, addrptr->ai_protocol)) < 0) {
            continue;
        }
        if (connect(sockfd, addrptr->ai_addr, addrptr->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
    }
    freeaddrinfo(res);

    str_cli(stdin, sockfd);
}

static char *gf_time(void) {
    static char str[30];
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        err_sys("clock_gettime error");
    }
    char *ptr = ctime(&ts.tv_sec);
    strncpy(str, &ptr[11], sizeof(str) - 1);
    str[sizeof(str) - 1] = 0;
    snprintf(str + 8, sizeof(str) - 8, ".%06ld", ts.tv_nsec / 1000);
    return str;
}

void setFDUnblock(int fd) {
    int val = fcntl(fd, F_GETFL);
    if (val < 0) {
        err_sys("fcntl error");
    }
    if (fcntl(fd, F_SETFL, val | O_NONBLOCK) < 0) {
        err_sys("fcntl error");
    }
}

void str_cli(FILE *fp, int sockfd) {
    setFDUnblock(sockfd);
    setFDUnblock(STDIN_FILENO);
    setFDUnblock(STDOUT_FILENO);

    char to[MAXLINE], fr[MAXLINE];
    char *toiptr, *tooptr, *friptr, *froptr;
    toiptr = tooptr = to;
    friptr = froptr = fr;

    int stdineof = 0;
    int maxfdp1 = max_int(max_int(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;
    int n, nwritten;

    fd_set rset, wset;
    for (;;) {
        FD_ZERO(&rset);
        FD_ZERO(&wset);

        if (stdineof == 0 && toiptr < &to[MAXLINE]) {
            FD_SET(STDIN_FILENO, &rset); // read from stdin
        }
        if (friptr < &fr[MAXLINE]) {
            FD_SET(sockfd, &rset); // read from socket
        }
        if (tooptr != toiptr) {
            FD_SET(sockfd, &wset); // write to socket
        }
        if (froptr != friptr) {
            FD_SET(STDOUT_FILENO, &wset); // write to stdout
        }

        if (select(maxfdp1, &rset, &wset, NULL, NULL) < 0) {
            if (errno == EINTR) {
                continue;
            }
            err_sys("select error");
        }

        if (FD_ISSET(STDIN_FILENO, &rset)) {
            if ((n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) {
                if (errno != EWOULDBLOCK) {
                    err_sys("read error on stdin");
                }
            } else if (n == 0) {
                // read eof
                fprintf(stderr, "%s: EOF on stdin\n", gf_time());
                stdineof = 1;
                if (tooptr == toiptr) { // data send complete, so we should shutdown socket
                    shutdown(sockfd, SHUT_WR);
                }
            } else {
                fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
                toiptr += n;
                FD_SET(sockfd, &wset); // try and write to socket below
            }
        }
        if (FD_ISSET(sockfd, &rset)) {
            if ((n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0) {
                if (errno != EWOULDBLOCK) {
                    err_sys("read error on socket");
                }
            } else if (n == 0) {
                // read eof on socket
                fprintf(stderr, "%s: EOF on socket\n", gf_time());
                if (stdineof) {
                    // normal termination
                    return;
                } else {
                    err_quit("str_cli: server terminated preaturely");
                }
            } else {
                fprintf(stderr, "%s: read %d bytes from socket\n", gf_time(), n);
                friptr += n;
                FD_SET(STDOUT_FILENO, &wset); // try and write to stdout below
            }
        }
        if (FD_ISSET(STDOUT_FILENO, &wset) && ((n = friptr - froptr) > 0)) {
            if ((nwritten = write(STDOUT_FILENO, froptr, n)) < 0) {
                if (errno != EWOULDBLOCK) {
                    err_sys("write error to stdout");
                }
            } else {
                fprintf(stderr, "%s: wrote %d bytes to stdout\n", gf_time(), nwritten);
                froptr += nwritten;
                if (froptr == friptr) {
                    froptr = friptr = fr;
                }
            }
        }
        if (FD_ISSET(sockfd, &wset) && ((n = toiptr - tooptr) > 0)) {
            if ((nwritten = write(sockfd, tooptr, n)) < 0) {
                if (errno != EWOULDBLOCK) {
                    err_sys("write error to socket");
                }
            } else {
                fprintf(stderr, "%s: wrote %d bytes to socket\n", gf_time(), nwritten);
                tooptr += nwritten;
                if (tooptr == toiptr) {
                    tooptr = toiptr = to;
                    if (stdineof) {
                        shutdown(sockfd, SHUT_WR);
                    }
                }
            }
        }
    }
}
