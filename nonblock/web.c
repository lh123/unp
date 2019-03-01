#include "web.h"

int min(int a, int b) {
    return a < b ? a : b;
}

int main(int argc, char **argv) {
    if (argc < 5) {
        err_quit("usage: web <#conns> <hostname> <homepage> <file1> ...");
    }
    int maxnconn = atoi(argv[1]);
    nfiles = min(argc - 4, MAXFILES);
    int i;
    for (i = 0; i < nfiles; i++) {
        file[i].f_name = argv[i + 4];
        file[i].f_host = argv[2];
        file[i].f_flags = 0;
    }
    printf("nfiles = %d\n", nfiles);

    home_page(argv[2], argv[3]);
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    maxfd = -1;
    nlefttoread = nlefttoconn = nfiles;
    nconn = 0;

    char buf[MAXLINE];
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
            start_connect(&file[i]);
            nconn++;
            nlefttoconn--;
        }

        fd_set rs = rset, ws = wset;
        int n = select(maxfd + 1, &rs, &ws, NULL, NULL);
        if (n < 0) {
            err_sys("select error");
        }
        for (i = 0; i < nfiles; i++) {
            int flags = file[i].f_flags;
            if (flags == 0 || flags & F_DONE) {
                continue;
            }
            int fd = file[i].f_fd;
            if ((flags & F_CONNECTING) && (FD_ISSET(fd, &rs) || FD_ISSET(fd, &ws))) {
                int error;
                socklen_t len = sizeof(error);
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                    err_ret("nonblocking connect failed for %s", file[i].f_name);
                }
                
                // connection estabished
                printf("connection established for %s\n", file[i].f_name);
                FD_CLR(fd, &wset);
                write_get_cmd(&file[i]);
            } else if ((flags & F_READING) && FD_ISSET(fd, &rs)) {
                if ((n = read(fd, buf, sizeof(buf))) < 0) {
                    err_sys("read error");
                } else if (n == 0) {
                    printf("end-of-file on %s\n", file[i].f_name);
                    close(fd);
                    file[i].f_flags = F_DONE;
                    FD_CLR(fd, &rset);
                    nconn--;
                    nlefttoread--;
                } else {
                    printf("read %d bytes from %s\n", n, file[i].f_name);
                }
            }
        } 
    }
}
