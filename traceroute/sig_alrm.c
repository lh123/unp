#include "trace.h"

int gotalarm;

void sig_alrm(int signo) {
    gotalarm = 1;
}
