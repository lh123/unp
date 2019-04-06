#if !defined(_SEMAPHORE_H)
#define _SEMAPHORE_H

#include "unp.h"
#include <fcntl.h>
#include <sys/stat.h>

typedef struct {
    int sem_fd[2];
    int sem_magic;
} my_sem_t;

#define MY_SEM_MAGIC 0x89674523

#define MY_SEM_FAILED ((my_sem_t *)-1)

my_sem_t *my_sem_open(const char *pathname, int oflag, ...);
int my_sem_close(my_sem_t *sem);
int my_sem_unlink(const char *pathname);
int my_sem_post(my_sem_t *sem);
int my_sem_wait(my_sem_t *sem);

#endif // _SEMAPHORE_H
