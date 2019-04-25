#include "unp.h"
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: shmunlink <name>");
    }
    if (shm_unlink(argv[1]) < 0) {
        err_sys("shm_unlink error");
    }
}
