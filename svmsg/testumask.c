#include "unp.h"
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
int main(int argc, char **argv) {
    if (msgget(IPC_PRIVATE, 0666 | IPC_CREAT | IPC_EXCL) < 0) {
        err_sys("msgget error");
    }
    unlink("/tmp/fifo.1");
    if (mkfifo("/tmp/fifo.1", 0666) < 0) {
        err_sys("mkfifo error");
    }
}
