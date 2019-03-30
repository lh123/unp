#include "unp.h"

#include <pthread.h>

#define MAXNITEMS   1000000
#define MAXNTHREADS 100

#define ERR_EXIT(func, ...) do { \
                                int err; \
                                if ((err = func(__VA_ARGS__)) != 0) {\
                                    err_exit(err, #func " error"); \
                                } \
                            } while (0);

// globals shared by threads
int nitems;                 // read-only by producer and consumer
int buff[MAXNITEMS];

struct {
    pthread_mutex_t mutex;
    int nput;               // next index to store
    int nval;               // next value to store
} put = {
    PTHREAD_MUTEX_INITIALIZER
};

struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready;             // number ready for consumer
} nready = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER
};

int min(int a, int b) {
    return a < b ? a : b;
}

void *produce(void *arg), *consume(void *arg);

int main(int argc, char **argv) {
    if (argc != 3) {
        err_quit("usage: prodcons2 <#items> <#threads>");
    }
    nitems = min(atoi(argv[1]), MAXNITEMS);
    int nthreads = min(atoi(argv[2]), MAXNTHREADS);
    ERR_EXIT(pthread_setconcurrency, nthreads);
    int i;
    int count[MAXNTHREADS];
    pthread_t tid_produce[MAXNTHREADS], tid_consume;
    for (i = 0; i < nthreads; i++) {
        count[i] = 0;
        ERR_EXIT(pthread_create, &tid_produce[i], NULL, &produce, &count[i]);
    }
    ERR_EXIT(pthread_create, &tid_consume, NULL, &consume, NULL);

    for (i = 0; i < nthreads; i++) {
        ERR_EXIT(pthread_join, tid_produce[i], NULL);
        printf("count[%d] = %d\n", i, count[i]);
    }
    ERR_EXIT(pthread_join, tid_consume, NULL);
}

void cosume_wait(int i) {
    for (;;) {
        ERR_EXIT(pthread_mutex_lock, &put.mutex);
        if (i < put.nput) {
            ERR_EXIT(pthread_mutex_unlock, &put.mutex);
            return;
        }
        ERR_EXIT(pthread_mutex_unlock, &put.mutex);
    }
}

void *produce(void *arg) {
    for (;;) {
        ERR_EXIT(pthread_mutex_lock, &put.mutex);
        if (put.nput >= nitems) {
            ERR_EXIT(pthread_mutex_unlock, &put.mutex);
            return NULL;
        }
        buff[put.nput] = put.nval;
        put.nput++;
        put.nval++;
        ERR_EXIT(pthread_mutex_unlock, &put.mutex);

        ERR_EXIT(pthread_mutex_lock, &nready.mutex);
        if (nready.nready == 0) {
            ERR_EXIT(pthread_cond_signal, &nready.cond);
        }
        nready.nready++;
        ERR_EXIT(pthread_mutex_unlock, &nready.mutex);
        *(int *)arg += 1;
    }
}

void *consume(void *arg) {
    int i;
    for (i = 0; i < nitems; i++) {
        ERR_EXIT(pthread_mutex_lock, &nready.mutex);
        while (nready.nready == 0) {
            ERR_EXIT(pthread_cond_wait, &nready.cond, &nready.mutex);
        }
        nready.nready--;
        ERR_EXIT(pthread_mutex_unlock, &nready.mutex);

        if (buff[i] != i) {
            printf("buff[%d] = %d\n", i, buff[i]);
        }
    }
    return NULL;
}
