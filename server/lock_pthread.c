#include "unp.h"
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

static pthread_mutex_t *mptr; 

void my_lock_init(char *pathname) {
    int fd = open("/dev/zero", O_RDWR, 0);
    mptr = mmap(NULL, sizeof(pthread_mutex_t), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (mptr == NULL) {
        err_sys("mmap error");
    }
    close(fd);
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mptr, &mattr);
}

void my_lock_wait(void) {
    pthread_mutex_lock(mptr);
}

void my_lock_release(void) {
    pthread_mutex_unlock(mptr);
}
