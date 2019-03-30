#include "unp.h"
#include "pthread_rwlock.h"

static void rwlock_cancelrdwait(void *arg) {
    my_pthread_rwlock_t *rw = arg;
    rw->rw_nwaitreaders--;
    pthread_mutex_unlock(&rw->rw_mutex);
}

int my_pthread_rwlock_rdlock(my_pthread_rwlock_t *rw) {
    if (rw->rw_magic != MY_RW_MAGIC) {
        return EINVAL;
    }
    int result;

    if ((result = pthread_mutex_lock(&rw->rw_mutex)) != 0) {
        return result;
    }

    // rw_refcount 小于0 表示有线程持有写锁
    // rw_nwaitwriters 大于0 表示有线程正在等待获取写锁
    while (rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0) {
        rw->rw_nwaitreaders++;
        pthread_cleanup_push(rwlock_cancelrdwait, (void *)rw);
        result = pthread_cond_wait(&rw->rw_condreaders, &rw->rw_mutex);
        pthread_cleanup_pop(0);
        rw->rw_nwaitreaders--;
        if (result != 0) { // pthread_cond_wait error
            break;
        }
    }
    if (result == 0) {
        rw->rw_refcount++; // 当前线程获得读锁，rw_refcount自增
    }
    pthread_mutex_unlock(&rw->rw_mutex);
    return result;
}
