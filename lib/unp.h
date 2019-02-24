#if !defined(_APUE_H)
#define _APUE_H

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#define MAXLINE 4096

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

#define LISTENQ 8
#define SERV_PORT 9877
#define MAXFD 64
#define bzero(ptr, n) memset((ptr), 0, (n))

typedef struct sockaddr SA;
typedef void Sigfunc(int signo);

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

#endif