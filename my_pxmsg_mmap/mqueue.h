#if !defined(_MQUEUE_H)
#define _MQUEUE_H
#include "unp.h"
#include <pthread.h>

typedef struct mq_info *mqd_t;

struct mq_attr {
    long mq_flags;
    long mq_maxmsg;
    long mq_msgsize;
    long mq_curmsgs;
};

struct mq_hdr {
    struct mq_attr mqh_attr;
    long mqh_head;
    long mqh_free;
    long mqh_nwait;
    pid_t mqh_pid;
    struct sigevent mqh_event;
    pthread_mutex_t mqh_lock;
    pthread_cond_t mqh_wait;
};

struct msg_hdr {
    long msg_next;
    ssize_t msg_len;
    unsigned int msg_prio;
};

struct mq_info {
    struct mq_hdr *mqi_hdr;
    long mqi_magic;
    int mqi_flags;
};

#define MQI_MAGIC 0x98765432
#define MSGSIZE(i) (((i) + (sizeof(long) - 1)) & ~(sizeof(long) - 1))

mqd_t mq_open(const char *pathname, int oflag, ...);
int mq_close(mqd_t mqd);
int mq_unlink(const char *pathname);
int mq_notify(mqd_t mqd, const struct sigevent *notification);
int mq_getattr(mqd_t mqd, struct mq_attr *mqstat);
int mq_setattr(mqd_t mqd, struct mq_attr *mqstat, struct mq_attr *omqstat);
int mq_send(mqd_t mqd, const char *ptr, size_t len, unsigned int prio);
ssize_t mq_receive(mqd_t mqd, char *ptr, size_t maxlen, unsigned int *prio);

#endif // _MQUEUE_H
