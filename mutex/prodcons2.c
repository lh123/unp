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

int nitems;
struct {
    pthread_mutex_t mutex;
    int buff[MAXNITEMS];
    int nput;
    int nval;
} shared = {
    PTHREAD_MUTEX_INITIALIZER
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
    for (i = 0; i < nthreads; i++) {
        ERR_EXIT(pthread_join, tid_produce[i], NULL);
        printf("count[%d] = %d\n", i, count[i]);
    }

    ERR_EXIT(pthread_create, &tid_consume, NULL, &consume, NULL);
    ERR_EXIT(pthread_join, tid_consume, NULL);
}

void *produce(void *arg) {
    for (;;) {
        ERR_EXIT(pthread_mutex_lock, &shared.mutex);
        if (shared.nput >= nitems) {
            ERR_EXIT(pthread_mutex_unlock, &shared.mutex);
            return NULL;
        }
        shared.buff[shared.nput] = shared.nval;
        shared.nput++;
        shared.nval++;
        ERR_EXIT(pthread_mutex_unlock, &shared.mutex);
        *(int *)arg += 1;
    }
}

void *consume(void *arg) {
    int i;
    for (i = 0; i < nitems; i++) {
        if (shared.buff[i] != i) {
            printf("buff[%d] = %d\n", i, shared.buff[i]);
        }
    }
    return NULL;
}
