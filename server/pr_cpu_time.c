#include "unp.h"
#include <sys/resource.h>

void pr_cpu_time(void) {
    struct rusage myusage, childusage;

    if (getrusage(RUSAGE_SELF, &myusage) < 0) {
        err_sys("getrusage error");
    }
    if (getrusage(RUSAGE_CHILDREN, &childusage) < 0) {
        err_sys("getrusage error");
    }

    double user = myusage.ru_utime.tv_sec + myusage.ru_utime.tv_usec / 1000000.0;
    user += childusage.ru_utime.tv_sec + childusage.ru_utime.tv_usec / 1000000.0;
    double sys = myusage.ru_stime.tv_sec +  myusage.ru_stime.tv_usec / 1000000.0;
    sys += childusage.ru_stime.tv_sec +  childusage.ru_stime.tv_usec / 1000000.0;

    printf("\nuser time = %g, sys time = %g\n", user, sys);
}
