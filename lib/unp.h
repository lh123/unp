#if !defined(_APUE_H)
#define _APUE_H

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE     4096
#define MAXFD       64
#define BUFFSIZE    8192

#define UNIXSTR_PATH    "/tmp/unix.str"
#define UNIXDG_PATH     "/tmp/unix.dg"
#define FILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define SVMSG_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define SVSEM_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define SVSHM_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define LISTENQ     8
#define SERV_PORT   9877
#define SERV_PORT_STR "9877"

#define bzero(ptr, n) memset((ptr), 0, (n))

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

typedef struct sockaddr SA;
typedef void Sigfunc(int signo);
typedef void Sigfunc_rt(int signo, siginfo_t *info, void *context);

extern int daemon_proc;

void __attribute__((format(printf, 1, 2))) err_ret(const char *fmt, ...);
void __attribute__((noreturn, format(printf, 1, 2))) err_sys(const char *fmt, ...);
void __attribute__((format(printf, 2, 3))) err_cont(int error, const char *fmt, ...);
void __attribute__((noreturn, format(printf, 2, 3))) err_exit(int error, const char *fmt, ...);
void __attribute__((noreturn, format(printf, 1, 2))) err_dump(const char *fmt, ...);
void __attribute__((format(printf, 1, 2))) err_msg(const char *fmt, ...);
void __attribute__((noreturn, format(printf, 1, 2))) err_quit(const char *fmt, ...);

extern int log_to_stderr;

void log_open(const char *ident, int option, int facility);
void __attribute__((format(printf, 1, 2))) log_ret(const char *fmt, ...);
void __attribute__((noreturn, format(printf, 1, 2))) log_sys(const char *fmt, ...);
void __attribute__((format(printf, 2, 3))) log_cont(int error, const char *fmt, ...);
void __attribute__((noreturn, format(printf, 2, 3))) log_exit(int error, const char *fmt, ...);
void __attribute__((format(printf, 1, 2))) log_msg(const char *fmt, ...);
void __attribute__((noreturn, format(printf, 1, 2))) log_quit(const char *fmt, ...);

char *sock_ntop(const struct sockaddr *sockaddr, socklen_t addrlen);

ssize_t readn(int fd, void *ptr, size_t n);
ssize_t writen(int fd, const void *ptr, size_t n);
ssize_t readline(int fd, void *ptr, size_t maxlen);
ssize_t readlinewithbuf(int fd, void *vptr, size_t maxlen);

int daemon_init(const char *pname, int facility);

int connect_timeo(int sockfd, const struct sockaddr *saptr, socklen_t salen, int sec);

int readable_timeo(int fd, int sec);

ssize_t read_fd(int fd, void *ptr, size_t nbytes, int *recvfd);
ssize_t write_fd(int fd, void *ptr, size_t nbytes, int sendfd);

struct addrinfo *host_serv(const char *hostname, const char *service, int family, int socktype);

int tcp_listen(const char *host, const char *service);
int tcp_connect(const char *host, const char *service);
int udp_server(const char *host, const char *serv, socklen_t *addrlenp);
int udp_client(const char *host, const char *service, struct sockaddr *addr, socklen_t *addrlen);

int connect_nonb(int sockfd, const SA *saptr, socklen_t salen, int nsec);

int family_to_level(int family);
int sockfd_to_family(int sockfd);

int mcast_join(int sockfd, const struct sockaddr *grp, socklen_t grplen, const char *ifname, u_int ifindex);
int mcast_leave(int sockfd, const struct sockaddr *grp, socklen_t grplen);
int mcast_block_source(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen);
int mcast_unblock_source(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen);
int mcast_join_source_group(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen, const char *ifname, u_int ifindex);
int mcast_leave_source_group(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen);
int mcast_set_if(int sockfd, const char *ifname, u_int ifindex);
int mcast_set_loop(int sockfd, int onoff);
int mcast_set_ttl(int sockfd, int val);
int mcast_get_if(int sockfd);
int mcast_get_loop(int sockfd);
int mcast_get_ttl(int sockfd);

char *px_ipc_name(const char *name);
Sigfunc_rt *signal_rt(int signo, Sigfunc_rt *func, sigset_t *mask);

#define read_lock(fd, offset, whence, len) \
            lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define readw_lock(fd, offset, whence, len) \
            lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define write_lock(fd, offset, whence, len) \
            lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define writew_lock(fd, offset, whence, len) \
            lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define un_lock(fd, offset, whence, len) \
            lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))
#define is_read_lockable(fd, offset, whence, len) \
            (!lock_test((fd), F_RDLCK, (offset), (whence), (len)))
#define is_write_lockable(fd, offset, whence, len) \
            (!lock_test((fd), F_WRLCK, (offset), (whence), (len)))

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len);
pid_t lock_test(int fd, int type, off_t offset, int whence, off_t len);

#endif
