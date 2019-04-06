#include "semaphore.h"

int my_sem_post(my_sem_t *sem) {
    if (sem->sem_magic != MY_SEM_MAGIC) {
        errno = EINVAL;
        return -1;
    }
    int n;
    if ((n = pthread_mutex_lock(&sem->sem_mutex)) != 0) {
        errno = n;
        return -1;
    }
    if (sem->sem_count == 0) {
        pthread_cond_signal(&sem->sem_cond);
    }
    sem->sem_count++;
    pthread_mutex_unlock(&sem->sem_mutex);
    return 0;
}
