#include "unp.h"

void client(int readfd, int writefd), server(int readfd, int writefd);

int main(int argc, char **argv) {
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) < 0) {
        err_sys("pipe error");
    }
    if (pipe(pipe2) < 0) {
        err_sys("pipe error");
    }

    pid_t childpid = fork();
    if (childpid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);
        server(pipe1[0], pipe2[1]);
        exit(0);
    }
    close(pipe1[0]); // close pipe1 read
    close(pipe2[1]); // close pipe2 write

    client(pipe2[0], pipe1[1]);
    
    if (waitpid(-1, NULL, 0) < 0) {
        err_sys("waitpid error");
    }
}
