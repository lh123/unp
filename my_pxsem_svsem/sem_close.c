#include "semaphore.h"

int my_sem_close(my_sem_t *sem) {
    if (sem->sem_magic != MY_SEM_MAGIC) {
        errno = EINVAL;
        return -1;
    }
    sem->sem_magic = 0;
    free(sem);
    return 0;
}
