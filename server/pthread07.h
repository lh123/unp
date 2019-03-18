#ifndef _PTHREAD_07_H
#define _PTHREAD_07_H

#include "unp.h"
#include <pthread.h>

typedef struct {
    pthread_t thread_tid;
    long thread_count;
} Thread;

extern Thread *tptr;

extern int listenfd, nthreads;
extern pthread_mutex_t mlock;

void thread_make(int i);

#endif // _PTHREAD_07_H
