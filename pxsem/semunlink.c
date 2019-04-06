#include "unp.h"
#include <semaphore.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: semunlink <name>");
    }
    if (sem_unlink(argv[1]) < 0) {
        err_sys("sem_unlink error");
    }
}
