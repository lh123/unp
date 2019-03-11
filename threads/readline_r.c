#include "unp.h"
#include <pthread.h>

#ifdef _PTHREAD_H
static pthread_key_t rl_key;
static pthread_once_t rl_once = PTHREAD_ONCE_INIT;


static void readline_destructor(void *ptr) {
    free(ptr);
}

static void readline_once(void) {
    int err = pthread_key_create(&rl_key, readline_destructor);
    if (err != 0) {
        err_exit(err, "pthread_key_create error");
    }
}
#endif

typedef struct {
    int rl_cnt;
    char *rl_bufptr;
    char rl_buf[MAXLINE];
} Rline;

#ifndef _PTHREAD_H
static Rline *tsd = NULL;
#endif

static ssize_t my_read(Rline *tsd, int fd, char *ptr) {
    if (tsd->rl_cnt <= 0) {
again:
        if ((tsd->rl_cnt = read(fd, tsd->rl_buf, MAXLINE)) < 0) {
            if (errno == EINTR) {
                goto again;
            }
            return -1;
        } else if (tsd->rl_cnt == 0) {
            return 0;
        }
        tsd->rl_bufptr = tsd->rl_buf;
    }
    tsd->rl_cnt--;
    *ptr = *tsd->rl_bufptr++;
    return 1;
}

// 线程安全的readline
ssize_t readline_r(int fd, void *vptr, size_t maxlen) {
#ifdef _PTHREAD_H
    int err = pthread_once(&rl_once, readline_once);
    if (err != 0) {
        err_exit(err, "pthread_once error");
    }
    Rline *tsd;
    if ((tsd = pthread_getspecific(rl_key)) == NULL) {
        tsd = calloc(1, sizeof(Rline));
        pthread_setspecific(rl_key, tsd);
    }
#else
    if (tsd == NULL) {
        tsd = calloc(1, sizeof(Rline));
    }
#endif
    char *ptr = vptr;
    size_t n;
    char c;
    for (n = 1; n < maxlen; n++) {
        ssize_t rc = my_read(tsd, fd, &c);
        if (rc == 1) {
            *ptr++ = c;
            if (c == '\n') {
                break;
            }
        } else if (rc == 0) {
            *ptr = 0;
            return n - 1;
        } else {
            return -1;
        }
    }
    *ptr = 0;
    return n;
}
