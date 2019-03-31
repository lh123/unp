#include "unp.h"
#include <fcntl.h>
#include <time.h>

#define STR1(x) #x
#define STR(x) STR1(x)
#define ERR_SYS(func) if ((func) < 0) { \
            err_sys("(" __FILE__ ", " STR(__LINE__) "): " #func " error"); \
        }

const char *gf_time(void) {
    static char str[20];
    time_t tim = time(NULL);
    strftime(str, sizeof(str), "%H:%M:%S", localtime(&tim));
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    size_t len = strlen(str);
    snprintf(str + len, sizeof(str) - len, ".%06ld", ts.tv_nsec / 1000);
    return str;
}

int main(int argc, char **argv) {
    int fd = open("test1.out", O_RDWR | O_CREAT, FILEMODE);
    if (fd < 0) {
        err_sys("open error");
    }
    ERR_SYS(read_lock(fd, 0, SEEK_SET, 0));
    printf("%s: parent has write lock\n", gf_time());
    pid_t pid;

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        // first child
        sleep(1);
        printf("%s: first child tries to obtain write lock\n", gf_time());
        ERR_SYS(writew_lock(fd, 0, SEEK_SET, 0));
        printf("%s: first child obtains write lock\n", gf_time());
        sleep(2);
        ERR_SYS(un_lock(fd, 0, SEEK_SET, 0));
        printf("%s: first child releases write lock\n", gf_time());
        exit(0);
    }

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid == 0) {
        // second child
        sleep(3);
        printf("%s: second child tries to obtain read lock\n", gf_time());
        ERR_SYS(readw_lock(fd, 0, SEEK_SET, 0));
        printf("%s: second child obtains read lock\n", gf_time());
        sleep(4);
        ERR_SYS(un_lock(fd, 0, SEEK_SET, 0));
        printf("%s: second child releases read lock\n", gf_time());
        exit(0);
    }
    // parent
    sleep(5);
    ERR_SYS(un_lock(fd, 0, SEEK_SET, 0));
    printf("%s: parent releases write lock\n", gf_time());
    wait(NULL);
    wait(NULL);
}
