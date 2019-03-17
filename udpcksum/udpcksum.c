#include "udpcksum.h"
#include <setjmp.h>

static sigjmp_buf jmpbuf;
static int canjump;
static void sig_alrm(int signo);

void test_udp(void) {
    struct sigaction act;
    act.sa_handler = sig_alrm;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGALRM, &act, NULL) < 0) {
        err_sys("sigaction error");
    }

    volatile int nsent = 0, timeout = 3;
    if (sigsetjmp(jmpbuf, 1)) {
        if (nsent >= 3) {
            err_quit("no response");
        }
        printf("timeout\n");
        timeout *= 2;
    }
    canjump = 1;

    send_dns_query();
    nsent++;

    alarm(timeout);
    struct udpiphdr *ui = udp_read();
    canjump = 0;
    alarm(0);
    if (ui->udp.uh_sum == 0) {
        printf("UDP checksums off\n");
    } else {
        printf("UDP checksums on\n");
    }
    if (verbose) {
        printf("received UDP checksum = %x\n", ntohs(ui->udp.uh_sum));
    }
}

static void sig_alrm(int signo) {
    if (canjump == 0) {
        return;
    }
    siglongjmp(jmpbuf, 1);
}
