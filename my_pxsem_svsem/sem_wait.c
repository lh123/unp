#include "semaphore.h"

int my_sem_wait(my_sem_t *sem) {
    if (sem->sem_magic != MY_SEM_MAGIC) {
        errno = EINVAL;
        return -1;
    }
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;
    op.sem_flg = 0;
    if (semop(sem->sem_semid, &op, 1) < 0) {
        return -1;
    }
    return 0;
}
