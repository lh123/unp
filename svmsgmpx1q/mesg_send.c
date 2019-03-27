#include "mesg.h"

#include <sys/msg.h>

ssize_t mesg_send(int id, struct mymesg *mptr) {
    return msgsnd(id, &mptr->mesg_type, mptr->mesg_len, 0);
}
