#include "unp.h"
#include "semaphore.h"
#include <stdarg.h>

my_sem_t *my_sem_open(const char *pathname, int oflag, ...) {
    unsigned int value;
    if (oflag & O_CREAT) {
        va_list ap;
        va_start(ap, oflag);
        mode_t mode = va_arg(ap, mode_t);
        value = va_arg(ap, unsigned int);
        va_end(ap);

        if (mkfifo(pathname, mode) < 0) {
            if (errno == EEXIST && (oflag & O_EXCL) == 0) {
                oflag &= ~O_CREAT;
            } else {
                return MY_SEM_FAILED;
            }
        }
    }
    my_sem_t *sem;
    if ((sem = malloc(sizeof(my_sem_t))) == NULL) {
        return MY_SEM_FAILED;
    }
    sem->sem_fd[0] = sem->sem_fd[1] = -1;

    if ((sem->sem_fd[0] = open(pathname, O_RDONLY | O_NONBLOCK)) < 0) {
        goto error;
    }
    if ((sem->sem_fd[1] = open(pathname, O_WRONLY | O_NONBLOCK)) < 0) {
        goto error;
    }
    int flags;
    if ((flags = fcntl(sem->sem_fd[0], F_GETFL)) < 0) {
        goto error;
    }
    flags &= ~O_NONBLOCK;
    if (fcntl(sem->sem_fd[0], F_SETFL, flags) < 0) {
        goto error;
    }
    if (oflag & O_CREAT) {
        int i;
        char c;
        for (i = 0; i < value; i++) {
            if (write(sem->sem_fd[1], &c, 1) != 1) {
                goto error;
            }
        }
    }
    sem->sem_magic = MY_SEM_MAGIC;
    return sem;

    int save_errno;
error:
    save_errno = errno;
    if (oflag & O_CREAT) {
        unlink(pathname);
    }
    close(sem->sem_fd[0]);
    close(sem->sem_fd[1]);
    free(sem);
    errno = save_errno;
    return MY_SEM_FAILED;
}
