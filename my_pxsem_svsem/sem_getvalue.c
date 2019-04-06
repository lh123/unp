#include "semaphore.h"

int my_sem_getvalue(my_sem_t *sem, int *pvalue) {
    if (sem->sem_magic != MY_SEM_MAGIC) {
        errno = EINVAL;
        return -1;
    }
    int val;
    if ((val = semctl(sem->sem_semid, 0, GETVAL)) < 0) {
        return -1;
    }
    *pvalue = val;
    return 0;
}
