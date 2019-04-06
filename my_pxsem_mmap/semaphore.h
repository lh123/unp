#if !defined(_SEMAPHORE_H)
#define _SEMAPHORE_H

#include "unp.h"
#include <pthread.h>
#include <sys/mman.h>

typedef struct {
    pthread_mutex_t sem_mutex;
    pthread_cond_t sem_cond;
    unsigned int sem_count;
    int sem_magic;
} my_sem_t;

#define MY_SEM_MAGIC 0x67458923

#define MY_SEM_FAILED ((my_sem_t *)-1)

my_sem_t *my_sem_open(const char *pathname, int oflag, ...);
int my_sem_close(my_sem_t *sem);
int my_sem_unlink(const char *pathname);
int my_sem_post(my_sem_t *sem);
int my_sem_wait(my_sem_t *sem);
int my_sem_trywait(my_sem_t *sem);
int my_sem_getvalue(my_sem_t *sem, int *pvalue);

#endif // _SEMAPHORE_H
