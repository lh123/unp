#include "unp.h"
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>

int main(int argc, char **argv) {
    key_t key;
    int id, i;
    struct shmid_ds buffer;
    char *ptr;

    if (argc != 2) {
        err_quit("usage: shmread <pathname>");
    }
    if ((key = ftok(argv[1], 0)) < 0) {
        err_quit("ftok error");
    }
    if ((id = shmget(key, 0, SVSHM_MODE)) < 0) {
        err_sys("shmget error");
    }
    if (shmctl(id, IPC_STAT, &buffer) < 0) {
        err_sys("shmctl error");
    }

    if ((ptr = shmat(id, NULL, 0)) == (char *)-1) {
        err_sys("shmat error");
    }
    for (i = 0; i < buffer.shm_segsz; i++) {
        if ((unsigned char)ptr[i] != i % 256) {
            err_ret("ptr[%d] = %d", i, ptr[i]);
        }
    }
}
