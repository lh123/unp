#if !defined(_SEMAPHORE_H)
#define _SEMAPHORE_H

#include <sys/sem.h>
#include <fcntl.h>
#include "unp.h"

typedef struct {
    int sem_semid;
    int sem_magic;
} my_sem_t;

#define MY_SEM_MAGIC    0x45678923
#define MY_SEM_FAILED   ((my_sem_t *)-1)
#define SEMVMX          32767
#define MAX_TRIES       10

my_sem_t *my_sem_open(const char *pathname, int oflag, ...);
int my_sem_close(my_sem_t *sem);
int my_sem_unlink(const char *pathname);
int my_sem_post(my_sem_t *sem);
int my_sem_wait(my_sem_t *sem);
int my_sem_trywait(my_sem_t *sem);
int my_sem_getvalue(my_sem_t *sem, int *pvalue);

#endif // _SEMAPHORE_H
