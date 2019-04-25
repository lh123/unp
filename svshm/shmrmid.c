#include "unp.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    int id;
    key_t key;

    if (argc != 2) {
        err_quit("usage: shmrmid <pathname>");
    }
    if ((key = ftok(argv[1], 0)) < 0) {
        err_sys("ftok error");
    }
    if ((id = shmget(key, 0, SVSHM_MODE)) < 0) {
        err_sys("shmget error");
    }
    if (shmctl(id, IPC_RMID, NULL) < 0) {
        err_sys("shmctl error");
    }
}
