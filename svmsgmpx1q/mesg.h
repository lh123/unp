#if !defined(_MSG_H)
#define _MSG_H

#include "unp.h"
#include <sys/stat.h>
#include <limits.h>

#define MAXMESGDATA (PIPE_BUF - 2 * sizeof(long))
#define MESGHDRSIZE (sizeof(struct mymesg) - MAXMESGDATA)

struct mymesg {
    long mesg_len;                  // #bytes in mesg_data, can be 0
    long mesg_type;                 // message type, must be > 0
    char mesg_data[MAXMESGDATA];
};

ssize_t mesg_send(int id, struct mymesg *mptr);
ssize_t mesg_recv(int id, struct mymesg *mptr);

#endif // _MSG_H
