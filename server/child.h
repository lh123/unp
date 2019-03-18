#ifndef _CHILD_H
#define _CHILD_H
#include "unp.h"

typedef struct {
    pid_t child_pid;    // process ID
    int child_pipefd;   // parent's stream pipe to/from child
    int child_status;   // 0 = ready
    long child_count;   // # connections handled
} Child;

extern Child *cptr;

pid_t child_make(int i, int listenfd);
void child_main(int i, int listenfd);

#endif // _CHILD_H