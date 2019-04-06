#include "unp.h"
#include "semaphore.h"

int my_sem_wait(my_sem_t *sem) {
    if (sem->sem_magic != MY_SEM_MAGIC) {
        errno = EINVAL;
        return -1;
    }
    char c;
    if (read(sem->sem_fd[0], &c, 1) == 1) {
        return 0;
    }
    return -1;
}
