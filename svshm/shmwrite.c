#include "unp.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    key_t key;
    int id, i;
    char *ptr;
    struct shmid_ds buff;
    
    if (argc != 2) {
        err_quit("usage: shmwrite <pathname>");
    }
    if ((key = ftok(argv[1], 0)) < 0) {
        err_sys("ftok error");
    }
    if ((id = shmget(key, 0, SVSHM_MODE)) < 0) {
        err_sys("shmget error");
    }
    if ((ptr = shmat(id, NULL, 0)) == (char *)-1) {
        err_sys("shmat error");
    }
    if (shmctl(id, IPC_STAT, &buff) < 0) {
        err_sys("shmctl error");
    }

    for (i = 0; i < buff.shm_segsz; i++) {
        ptr[i] = i % 256;
    }
}
