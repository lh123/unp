#include "unp.h"
#include "semaphore.h"

int my_sem_post(my_sem_t *sem) {
    if (sem->sem_magic != MY_SEM_MAGIC) {
        errno = EINVAL;
        return -1;
    }
    char c;
    if (write(sem->sem_fd[1], &c, 1) == 1) {
        return 0;
    }
    return -1;
}
