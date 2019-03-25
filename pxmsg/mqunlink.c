#include "unp.h"
#include <mqueue.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        err_quit("usage: mqunlink <name>");
    }
    if (mq_unlink(argv[1]) < 0) {
        err_sys("mq_unlink error");
    }
}
