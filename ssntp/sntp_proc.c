#include "sntp.h"
#include <time.h>

void sntp_proc(char *buf, ssize_t n, struct timespec *nowptr) {
    if (n < (ssize_t)sizeof(struct ntpdata)) {
        printf("\npacket too small: %ld bytes\n", n);
        return;
    }
    struct ntpdata *ntp = (struct ntpdata *)buf;
    int version = (ntp->status & VERSION_MASK) >> VERSION_SHIFT;
    int mode = ntp->status & MODE_MASK;
    printf("\nv%d, mode %d, strat %d, ", version, mode, ntp->stratum);
    if (mode == MODE_CLIENT) {
        printf("client\n");
        return;
    }
    uint32_t nsec = ntohl(ntp->xmt.int_part) - JAN_1970;
    uint32_t useci = ntohl(ntp->xmt.fraction);
    double usecf = useci / (double)(1UL << 32);
    useci = usecf * 1000000;

    struct timespec diff;
    diff.tv_sec = nowptr->tv_sec - nsec;
    if ((diff.tv_nsec = nowptr->tv_nsec / 1000 - useci) < 0) {
        diff.tv_sec--;
        diff.tv_nsec += 1000000;
    }
    useci = (diff.tv_sec * 1000000) + diff.tv_nsec;
    printf("clock difference = %d usec\n", useci);
}
