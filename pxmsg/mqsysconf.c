#include "unp.h"

long Sysconf(int name) {
    errno = 0;
    long ret = sysconf(name);
    if (ret < 0 && errno != 0) {
        err_sys("sysconf error");
    }
    return ret;
}

int main(int argc, char **argv) {
    printf("MQ_OPEN_MAX = %ld, MQ_PRIO_MAX = %ld\n", Sysconf(_SC_MQ_OPEN_MAX), Sysconf(_SC_MQ_PRIO_MAX));
} 
