#ifndef _PTHREAD_08_H
#define _PTHREAD_08_H

#include "unp.h"
#include <pthread.h>

typedef struct {
    pthread_t thread_tid;
    long thread_count;
} Thread;

extern Thread *tptr;

#define MAXNCLI 32
extern int clifd[MAXNCLI], iget, iput;
extern pthread_mutex_t clifd_mutex;
extern pthread_cond_t clifd_cond;

void thread_make(int i);

#endif // _PTHREAD_08_H