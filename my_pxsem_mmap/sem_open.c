#include "semaphore.h"
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>

#define MAX_TRIES 10

my_sem_t *my_sem_open(const char *pathname, int oflag, ...) {
    unsigned int value;
    int created = 0, i, fd;
    my_sem_t *sem = MY_SEM_FAILED;
again:
    if (oflag & O_CREAT) {
        va_list ap;
        va_start(ap, oflag);
        mode_t mode = va_arg(ap, mode_t);
        value = va_arg(ap, unsigned int);
        va_end(ap);
        fd = open(pathname, oflag | O_EXCL | O_RDWR, mode | S_IXUSR);
        if (fd < 0) {
            if (errno == EEXIST && (oflag & O_EXCL) == 0) {
                goto exists;
            } else {
                return MY_SEM_FAILED;
            }
        }
        created = 1;
        my_sem_t seminit;
        memset(&seminit, 0, sizeof(seminit));
        if (write(fd, &seminit, sizeof(seminit)) != sizeof(seminit)) {
            goto err;
        }
        sem = mmap(NULL, sizeof(my_sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (sem == MAP_FAILED) {
            goto err;
        }
        pthread_mutexattr_t mattr;
        if ((i = pthread_mutexattr_init(&mattr)) != 0) {
            goto pthreaderr;
        }
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        i = pthread_mutex_init(&sem->sem_mutex, &mattr);
        pthread_mutexattr_destroy(&mattr);
        if (i != 0) {
            goto pthreaderr;
        }
        pthread_cond_t cattr;
        if ((i = pthread_condattr_init(&cattr)) != 0) {
            goto pthreaderr;
        }
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
        i = pthread_cond_init(&sem->sem_cond, &cattr);
        pthread_cond_destroy(&cattr);
        if (i != 0) {
            goto pthreaderr;
        }
        if ((sem->sem_count = value) > sysconf(_SC_SEM_VALUE_MAX)) {
            errno = EINVAL;
            goto err;
        }
        if (fchmod(fd, mode) == -1) {
            goto err;
        }
        close(fd);
        sem->sem_magic = MY_SEM_MAGIC;
        return sem;
    }
exists:
    if ((fd = open(pathname, O_RDWR)) < 0) {
        if (errno == ENOENT && (oflag & O_CREAT)) {
            goto again;
        }
        goto err;
    }
    sem = mmap(NULL, sizeof(my_sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (sem == MAP_FAILED) {
        goto err;
    }
    struct stat statbuf;
    for (i = 0; i < MAX_TRIES; i++) {
        if (stat(pathname, &statbuf) == -1) {
            if (errno == ENOENT && (oflag & O_CREAT)) {
                close(fd);
                goto again;
            }
            goto err;
        }
        if ((statbuf.st_mode & S_IXUSR) == 0) {
            close(fd);
            sem->sem_magic = MY_SEM_MAGIC;
            return sem;
        }
        sleep(1);
    }
    errno = ETIMEDOUT;
    goto err;
pthreaderr:
    errno = i;

    int save_errno;
err:
    save_errno = errno;
    if (created) {
        unlink(pathname);
    }
    if (sem != MAP_FAILED) {
        munmap(sem, sizeof(my_sem_t));
    }
    close(fd);
    errno = save_errno;
    return MY_SEM_FAILED;
}
