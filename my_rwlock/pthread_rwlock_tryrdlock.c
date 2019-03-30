#include "unp.h"
#include "pthread_rwlock.h"

int my_pthread_rwlock_tryrdlock(my_pthread_rwlock_t *rw) {
    if (rw->rw_magic != MY_RW_MAGIC) {
        return EINVAL;
    }
    int result;

    if ((result = pthread_mutex_lock(&rw->rw_mutex)) != 0) {
        return result;
    }
    if (rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0) {
        result = EBUSY;
    } else {
        rw->rw_refcount++;
    }
    pthread_mutex_unlock(&rw->rw_mutex);
    return result;
}
