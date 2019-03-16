#include "unpicmpd.h"
#include <sys/un.h>
#include <sys/time.h>

void sock_bind_wild(int sockfd, int family);

void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen) {
    sock_bind_wild(sockfd, pservaddr->sa_family);
    int icmpfd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un sun;
    sun.sun_family = AF_LOCAL;
    strcpy(sun.sun_path, ICMPD_PATH);

    if (connect(icmpfd, (SA *)&sun, sizeof(sun)) < 0) {
        err_sys("connect error");
    }
    
    char c = '1';
    char control[CMSG_SPACE(sizeof(int))];
    struct cmsghdr *cmsg = (struct cmsghdr *)control;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    *(int *)CMSG_DATA(cmsg) = sockfd;

    struct iovec iov[1];
    iov[0].iov_base = &c;
    iov[0].iov_len = 1;

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    
    if (sendmsg(icmpfd, &msg, 0) < 0) {
        err_sys("sendmsg error");
    }

    char sendline[MAXLINE], recvline[MAXLINE + 1];
    ssize_t n = read(icmpfd, recvline, MAXLINE);
    if (n != 1 || recvline[0] != '1') {
        err_quit("err creating icmp socket, n = %d, char = %c", (int)n, recvline[0]);
    }
    fd_set rset;
    FD_ZERO(&rset);
    int maxfdp1 = ((sockfd > icmpfd) ? sockfd : icmpfd) + 1;

    while (fgets(sendline, MAXLINE, fp) != NULL) {
        if (sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen) < 0) {
            err_sys("sendto error");
        }
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        FD_SET(sockfd, &rset);
        FD_SET(icmpfd, &rset);
        if ((n = select(maxfdp1, &rset, NULL, NULL, &tv)) == 0) {
            fprintf(stderr, "socket timeout\n");
            continue;
        } else if (n < 0) {
            err_sys("select error");
        }
        if (FD_ISSET(sockfd, &rset)) {
            if ((n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) < 0) {
                err_sys("recvfrom error");
            }
            recvline[n] = 0;
            if (fputs(recvline, stdout) == EOF) {
                err_sys("fputs error");
            }
        }
        if (FD_ISSET(icmpfd, &rset)) {
            struct icmpd_err icmpd_err;
            if ((n = read(icmpfd, &icmpd_err, sizeof(icmpd_err))) == 0) {
                err_quit("ICMP daemon terminated");
            } else if (n != sizeof(icmpd_err)) {
                err_quit("n = %d, expected %d", (int)n, (int)sizeof(icmpd_err));
            } else if (n < 0) {
                err_sys("read error");
            }
            printf("ICMP error: dest = %s, %s, type = %d, code = %d\n", 
                    sock_ntop((SA *)&icmpd_err.icmpd_dest, icmpd_err.icmpd_len),
                    strerror(icmpd_err.icmpd_errno),
                    icmpd_err.icmpd_type, icmpd_err.icmpd_code);
        }
    }
}
