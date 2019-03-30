#if !defined(_PTHREAD_RWLOCK_H)
#define _PTHREAD_RWLOCK_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t rw_mutex;
    pthread_cond_t rw_condreaders;
    pthread_cond_t rw_condwriters;
    int rw_magic;
    int rw_nwaitreaders;
    int rw_nwaitwriters;
    int rw_refcount;
} my_pthread_rwlock_t;

#define MY_RW_MAGIC 0x19283764

#define MY_PTHREAD_RWLOCK_INITIALIZER { PTHREAD_MUTEX_INITIALIZER, \
            PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, \
            MY_RW_MAGIC, 0, 0, 0}

typedef int my_pthread_rwlockattr_t;

int my_pthread_rwlock_destory(my_pthread_rwlock_t *rw);
int my_pthread_rwlock_init(my_pthread_rwlock_t *rw, my_pthread_rwlockattr_t *attr);
int my_pthread_rwlock_rdlock(my_pthread_rwlock_t *rw);
int my_pthread_rwlock_tryrdlock(my_pthread_rwlock_t *rw);
int my_pthread_rwlock_trywrlock(my_pthread_rwlock_t *rw);
int my_pthread_rwlock_unlock(my_pthread_rwlock_t *rw);
int my_pthread_rwlock_wrlock(my_pthread_rwlock_t *rw);

#endif // _PTHREAD_RWLOCK_H
