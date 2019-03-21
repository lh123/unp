#include "unp.h"
#include <sys/stat.h>
#include <sys/ipc.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: ftok <pathname>");
    }
    struct stat state;
    if (stat(argv[1], &state) < 0) {
        err_sys("stat error");
    }
    printf("st_dev: %lx, st_ino: %lx, key: %x\n", (u_long)state.st_dev, (u_long)state.st_ino, ftok(argv[1], 0x57));
}
