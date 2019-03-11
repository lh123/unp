#include "unp.h"
#include <pthread.h>

#define MAXFILES    20
#define SERV        "80"

struct file {
    char *f_name;       // filename
    char *f_host;       // hostname or IP address
    int f_fd;           // descriptor
    int f_flags;        // F_xxx below
    pthread_t f_tid;    // thread ID
} file[MAXFILES];

#define F_CONNECTING    (1 << 0) // connect() in progress
#define F_READING       (1 << 1) // connect() complete; now reading
#define F_DONE          (1 << 2) // all done

#define CMD_GET "GET %s HTTP/1.0\r\n\r\n"

int nconn, nfiles, nlefttoconn, nlefttoread;

void *do_get_read(void *arg);
void home_page(const char *host, const char *fname);
void write_get_cmd(struct file *fptr);
int min(int a, int b);

static int exit_cnt;
static pthread_cond_t has_exit = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t exit_mutex = PTHREAD_MUTEX_INITIALIZER;

void wait_thread(void);
void notify_exit(void);

int main(int argc, char **argv) {
    if (argc < 5) {
        err_quit("usage: web <#conns> <IPaddr> <homepage> file1 ...");
    }
    errno = 0;
    int maxnconn = strtol(argv[1], NULL, 10);
    if (errno != 0) {
        err_sys("strtol error");
    }
    nfiles = min(argc - 4, MAXFILES);
    int i;
    for (i = 0; i < nfiles; i++) {
        file[i].f_name = argv[i + 4];
        file[i].f_host = argv[2];
        file[i].f_flags = 0;
    }
    printf("nfiles = %d\n", nfiles);
    home_page(argv[2], argv[3]);
    nlefttoconn = nlefttoread = nfiles;
    nconn = 0;

    while (nlefttoread > 0) {
        while (nconn < maxnconn && nlefttoconn > 0) {
            for (i = 0; i < nfiles; i++) {
                if (file[i].f_flags == 0) {
                    break;
                }
            }
            if (i == nfiles) {
                err_quit("nlefttoconn = %d but nothing found", nlefttoconn);
            }
            file[i].f_flags = F_CONNECTING;
            pthread_t tid;
            pthread_create(&tid, NULL, do_get_read, &file[i]);
            file[i].f_tid = tid;
            nconn++;
            nlefttoconn--;
        }
        wait_thread();
        nconn--;
        nlefttoread--;
    }
}

void *do_get_read(void *arg) {
    struct file *fptr = arg;
    int fd = tcp_connect(fptr->f_host, SERV);
    fptr->f_fd = fd;
    printf("do_get_read for %s, fd %d, thread %d\n", fptr->f_name, fptr->f_fd, (int)fptr->f_tid);
    write_get_cmd(fptr);

    char line[MAXLINE];
    for (;;) {
        ssize_t n = read(fd, line, MAXLINE);
        if (n == 0) {
            break;
        } else if (n < 0) {
            err_sys("read error");
        }
        printf("read %d bytes from %s\n", (int)n, fptr->f_name);
    }
    printf("end-of-file on %s\n", fptr->f_name);
    close(fd);
    fptr->f_flags = F_DONE;
    notify_exit();
    return fptr;
}

void home_page(const char *host, const char *fname) {
    int fd = tcp_connect(host, SERV);
    struct file homefile;
    homefile.f_fd = fd;
    homefile.f_name = "/";
    homefile.f_host = (char *)host;
    homefile.f_flags = 0;
    homefile.f_tid = 0;
    write_get_cmd(&homefile);
    
    char line[MAXLINE];
    for (;;) {
        ssize_t n = read(fd, line, MAXLINE);
        if (n == 0) {
            break;
        } else if (n < 0) {
            err_sys("read error");
        }
        printf("read %d bytes from %s\n", (int)n, homefile.f_name);
    }
    printf("end-of-file on %s\n", homefile.f_name);
    close(fd);
}

void write_get_cmd(struct file *fptr) {
    char line[MAXLINE];
    int n = snprintf(line, sizeof(line), CMD_GET, fptr->f_name);
    if (writen(fptr->f_fd, line, n) < 0) {
        err_sys("writen error");
    }
    printf("wrote %d bytes for %s\n", n, fptr->f_name);
    fptr->f_flags = F_READING;
}

int min(int a, int b) {
    return a < b ? a : b;
}

void wait_thread(void) {
    pthread_mutex_lock(&exit_mutex);
    while (exit_cnt == 0) {
        pthread_cond_wait(&has_exit, &exit_mutex);
    }
    exit_cnt--;
    pthread_mutex_unlock(&exit_mutex);
}

void notify_exit(void) {
    pthread_mutex_lock(&exit_mutex);
    exit_cnt++;
    pthread_cond_signal(&has_exit);
    pthread_mutex_unlock(&exit_mutex);
}
