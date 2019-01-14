#if !defined(_APUE_H)
#define _APUE_H

#if !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809
#endif

#define MAXLINE 4096

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#define LISTENQ 8

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

#endif