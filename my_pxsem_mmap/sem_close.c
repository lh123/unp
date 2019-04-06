#include "semaphore.h"

int my_sem_close(my_sem_t *sem) {
    if (sem->sem_magic != MY_SEM_MAGIC) {
        errno = EINVAL;
        return -1;
    }
    if (munmap(sem, sizeof(my_sem_t)) == -1) {
        return -1;
    }
    return 0;
}
