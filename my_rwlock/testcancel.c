#include "unp.h"
#include "pthread_rwlock.h"

#define ERR_EXIT(func, ...) do { \
            int err; \
            if ((err = (func)(__VA_ARGS__)) != 0) { \
                err_exit(err, #func " error"); \
            } \
        } while (0);

my_pthread_rwlock_t rwlock = MY_PTHREAD_RWLOCK_INITIALIZER;

pthread_t tid1, tid2;
void *thread1(void *), *thread2(void *);

int main(int argc, char **argv) {
    pthread_setconcurrency(2);
    ERR_EXIT(pthread_create, &tid1, NULL, &thread1, NULL);
    sleep(1);
    ERR_EXIT(pthread_create, &tid2, NULL, &thread2, NULL);

    void *status;
    ERR_EXIT(pthread_join, tid2, &status);
    if (status != PTHREAD_CANCELED) {
        printf("thread2 status = %p\n", status);
    }
    ERR_EXIT(pthread_join, tid1, &status);
    if (status != NULL) {
        printf("thread1 status = %p\n", status);
    }
    printf("rw_refcount = %d, rw_nwaitreaders = %d, rw_nwaitwriters = %d\n",
            rwlock.rw_refcount, rwlock.rw_nwaitreaders, 
            rwlock.rw_nwaitwriters);
    ERR_EXIT(my_pthread_rwlock_destory, &rwlock);
}

void *thread1(void *arg) {
    ERR_EXIT(my_pthread_rwlock_rdlock, &rwlock);
    printf("thread1() got a read lock\n");
    sleep(3);
    pthread_cancel(tid2);
    sleep(3);
    ERR_EXIT(my_pthread_rwlock_unlock, &rwlock);
    return NULL;
}

void *thread2(void *arg) {
    printf("thread2() trying to obtain a write lock\n");
    ERR_EXIT(my_pthread_rwlock_wrlock, &rwlock);
    printf("thread2() got a write lock\n");
    sleep(1);
    ERR_EXIT(my_pthread_rwlock_unlock, &rwlock);
    return NULL;
}
