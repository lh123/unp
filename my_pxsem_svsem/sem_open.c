#include "semaphore.h"
#include <stdarg.h>
#include <sys/stat.h>

my_sem_t *my_sem_open(const char *pathname, int oflag, ...) {
    int fd, semid, semflag, save_errno;
    union semun arg;
    struct sembuf initop;
    struct semid_ds seminfo;
    key_t key;
    my_sem_t *sem;

    semflag = SVMSG_MODE;
    semid = -1;
    if (oflag & O_CREAT) {
        va_list ap;
        va_start(ap, oflag);
        mode_t mode = va_arg(ap, mode_t);
        unsigned int value = va_arg(ap, unsigned int);
        va_end(ap);
        
        if ((fd = open(pathname, oflag, mode)) == -1) {
            return MY_SEM_FAILED;
        }
        close(fd);
        
        if ((key = ftok(pathname, 0)) == -1) {
            return MY_SEM_FAILED;
        }
        semflag = IPC_CREAT | (mode & 0777);
        if (oflag & O_EXCL) {
            semflag |= IPC_EXCL;
        }
        if ((semid = semget(key, 1, semflag | IPC_EXCL)) >= 0) {
            arg.val = 0;
            if (semctl(semid, 0, SETVAL, arg) == -1) {
                goto err;
            }
            if (value > SEMVMX) {
                errno = EINVAL;
                goto err;
            }
            initop.sem_num = 0;
            initop.sem_op = value;
            initop.sem_flg = 0;
            if (semop(semid, &initop, 1) == -1) {
                goto err;
            }
            goto finish;
        } else if (errno != EEXIST || (semflag & IPC_CREAT) != 0) {
            goto err;
        }
    }
    if ((key = ftok(pathname, 0)) == -1) {
        goto err;
    }
    if ((semid = semget(key, 0, semflag)) == -1) {
        goto err;
    }
    arg.buf = &seminfo;
    int i;
    for (i = 0; i < MAX_TRIES; i++) {
        if (semctl(semid, 0, IPC_STAT, arg) == -1) {
            goto err;
        }
        if (arg.buf->sem_otime != 0) {
            goto finish;
        }
        sleep(1);
    }
    errno = ETIMEDOUT;
err:
    save_errno = errno;
    if (semid != -1) {
        semctl(semid, 0, IPC_RMID);
    }
    errno = save_errno;
    return MY_SEM_FAILED;
finish:
    if ((sem = malloc(sizeof(my_sem_t))) == NULL) {
        goto err;
    }
    sem->sem_semid = semid;
    sem->sem_magic = MY_SEM_MAGIC;
    return sem;
}
