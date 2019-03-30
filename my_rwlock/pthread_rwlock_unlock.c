#include "unp.h"
#include "pthread_rwlock.h"

int my_pthread_rwlock_unlock(my_pthread_rwlock_t *rw) {
    if (rw->rw_magic != MY_RW_MAGIC) {
        return EINVAL;
    }
    int result;

    if ((result = pthread_mutex_lock(&rw->rw_mutex)) != 0) {
        return result;
    }
    if (rw->rw_refcount > 0) {
        rw->rw_refcount--;
    } else if (rw->rw_refcount == -1) {
        rw->rw_refcount = 0;
    } else {
        err_dump("rw_refcount = %d", rw->rw_refcount);
    }

    // give preference to waiting writers over waiting readers
    if (rw->rw_nwaitwriters > 0) {
        // 仍然有读线程，不应该发送信号，需要等到所有的锁都释放了才能获得写锁
        if (rw->rw_refcount == 0) {
            result = pthread_cond_signal(&rw->rw_condwriters);
        }
    } else if (rw->rw_nwaitreaders > 0) {
        result = pthread_cond_signal(&rw->rw_condreaders);
    }
    pthread_mutex_unlock(&rw->rw_mutex);
    return result;
}
