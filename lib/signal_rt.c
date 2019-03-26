#include "unp.h"

Sigfunc_rt *signal_rt(int signo, Sigfunc_rt *func, sigset_t *mask) {
    struct sigaction act, oact;
    act.sa_sigaction = func;
    act.sa_flags = SA_SIGINFO;
    act.sa_mask = *mask;
    if (signo == SIGALRM) {
        act.sa_flags |= SA_INTERRUPT;
    } else {
        act.sa_flags |= SA_RESTART;
    }
    if (sigaction(signo, &act, &oact) < 0) {
        return (Sigfunc_rt *)SIG_ERR;
    }
    return oact.sa_sigaction;
}
