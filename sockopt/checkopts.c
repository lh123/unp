#include "unp.h"
#include <netinet/tcp.h>
#include <sys/time.h>

union val {
    int i_val;
    long l_val;
    struct linger linger_val;
    struct timeval timeval_val;
} val;

static char *sock_str_flag(union val *, int);
static char *sock_str_int(union val *, int);
static char *sock_str_linger(union val *, int);
static char *sock_str_timeval(union val *, int);

#define MAKE_SOCK_OPT(level, name, func) {#name, level, name, func}

struct sock_opts {
    const char *opt_str;
    int opt_level;
    int opt_name;
    char *(*opt_val_str)(union val *, int);
} sock_opts[] = {
    MAKE_SOCK_OPT(SOL_SOCKET, SO_BROADCAST, sock_str_flag),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_DEBUG, sock_str_flag),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_DONTROUTE, sock_str_flag),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_ERROR, sock_str_int),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_KEEPALIVE, sock_str_flag),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_LINGER, sock_str_linger),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_OOBINLINE, sock_str_flag),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_RCVBUF, sock_str_int),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_SNDBUF, sock_str_int),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_RCVLOWAT, sock_str_int),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_SNDLOWAT, sock_str_int),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_RCVTIMEO, sock_str_timeval),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_SNDTIMEO, sock_str_timeval),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_REUSEADDR, sock_str_flag),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_REUSEPORT, sock_str_flag),
    MAKE_SOCK_OPT(SOL_SOCKET, SO_TYPE, sock_str_int),
    MAKE_SOCK_OPT(IPPROTO_IP, IP_TOS, sock_str_int),
    MAKE_SOCK_OPT(IPPROTO_IP, IP_TTL, sock_str_int),
    MAKE_SOCK_OPT(IPPROTO_IPV6, IPV6_DONTFRAG, sock_str_flag),
    MAKE_SOCK_OPT(IPPROTO_IPV6, IPV6_UNICAST_HOPS, sock_str_int),
    MAKE_SOCK_OPT(IPPROTO_IPV6, IPV6_V6ONLY, sock_str_flag),
    MAKE_SOCK_OPT(IPPROTO_TCP, TCP_MAXSEG, sock_str_int),
    MAKE_SOCK_OPT(IPPROTO_TCP, TCP_NODELAY, sock_str_flag),
    {NULL, 0, 0, NULL}
};

int main(int argc, char **argv) {
    int fd;
    socklen_t len;
    struct sock_opts *ptr;

    for (ptr = sock_opts; ptr->opt_str != NULL; ptr++) {
        printf("%s: ", ptr->opt_str);
        if (ptr->opt_val_str == NULL) {
            printf("(undefined)\n");
        } else {
            switch (ptr->opt_level) {
            case SOL_SOCKET:
            case IPPROTO_IP:
            case IPPROTO_TCP:
                if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    err_sys("socket error");
                }
                break;
            case IPPROTO_IPV6:
                if ((fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
                    err_sys("socket error");
                }
                break;
            case IPPROTO_SCTP:
                if ((fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) < 0) {
                    err_sys("socket error");
                }
                break;
            default:
                err_quit("Can't create fd for level %d\n", ptr->opt_level);
            }

            len = sizeof(val);
            if (getsockopt(fd, ptr->opt_level, ptr->opt_name, &val, &len) == -1){
                err_ret("getsockopt error");
            } else {
                printf("default = %s\n", (*ptr->opt_val_str)(&val, len));
            }
            close(fd);
        }
    }
}

static char strres[128];

static char *sock_str_flag(union val *ptr, int len) {
    if (len != sizeof(int)) {
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
    } else {
        snprintf(strres, sizeof(strres), "%s", (ptr->i_val == 0) ? "off" : "on");
    }
    return strres;
}

static char *sock_str_int(union val *ptr, int len) {
    if (len != sizeof(int)) {
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(long)", len);
    } else {
        snprintf(strres, sizeof(strres), "%d", ptr->i_val);
    }
    return strres;
}

static char *sock_str_linger(union val *ptr, int len) {
    if (len != sizeof(struct linger)) {
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(struct linger)", len);
    } else {
        snprintf(strres, sizeof(strres), "l_onoff = %d, l_linger = %d", ptr->linger_val.l_onoff, ptr->linger_val.l_linger);
    }
    return strres;
}

static char *sock_str_timeval(union val *ptr, int len) {
    if (len != sizeof(struct timeval)) {
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(struct timeval)", len);
    } else {
        snprintf(strres, sizeof(strres), "%d sec, %d usec", (int)ptr->timeval_val.tv_sec, (int)ptr->timeval_val.tv_usec);
    }
    return strres;
}