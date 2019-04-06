#include "semaphore.h"
#include <sys/stat.h>

int my_sem_unlink(const char *pathname) {
    key_t key;
    if ((key = ftok(pathname, 0)) == -1) {
        return -1;
    }
    if (unlink(pathname) == -1) {
        return -1;
    }
    int semid;
    if ((semid = semget(key, 1, SVMSG_MODE)) == -1) {
        return -1;
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        return -1;
    }
    return 0;
}
