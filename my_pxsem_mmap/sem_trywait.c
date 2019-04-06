#include "semaphore.h"

int my_sem_trywait(my_sem_t *sem) {
    if (sem->sem_magic != MY_SEM_MAGIC) {
        errno = EINVAL;
        return -1;
    }
    int n;
    if ((n = pthread_mutex_lock(&sem->sem_mutex)) != 0) {
        errno = n;
        return -1;
    }
    int rc;
    if (sem->sem_count > 0) {
        sem->sem_count--;
        rc = 0;
    } else {
        rc = -1;
        errno = EAGAIN;
    }
    pthread_mutex_unlock(&sem->sem_mutex);
    return rc;
}
