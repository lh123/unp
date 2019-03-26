#include "unp.h"
#include "mqueue.h"

#include <fcntl.h>

int mq_send(mqd_t mqd, const char *ptr, size_t len, unsigned int prio) {
    struct mq_info *mqinfo = mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        errno = EBADF;
        return -1;
    }
    struct mq_hdr *mqhdr = mqinfo->mqi_hdr;
    int8_t *mptr = (int8_t *)mqhdr;
    struct mq_attr *attr = &mqhdr->mqh_attr;

    int n;
    if ((n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) {
        errno = n;
        return -1;
    }
    if (len > attr->mq_msgsize) {
        errno = EMSGSIZE;
        goto err;
    }
    if (attr->mq_curmsgs == 0) {
        if (mqhdr->mqh_pid != 0 && mqhdr->mqh_nwait == 0) {
            struct sigevent *sigev = &mqhdr->mqh_event;
            if (sigev->sigev_notify == SIGEV_SIGNAL) {
                sigqueue(mqhdr->mqh_pid, sigev->sigev_signo, sigev->sigev_value);
            }
            mqhdr->mqh_pid = 0; // unregister
        }
    } else if (attr->mq_curmsgs >= attr->mq_maxmsg) {
        // queue is full
        if (mqinfo->mqi_flags & O_NONBLOCK) {
            errno = EAGAIN;
            goto err;
        }
        // wait for room for one message on the queue
        while (attr->mq_curmsgs >= attr->mq_maxmsg) {
            pthread_cond_wait(&mqhdr->mqh_wait, &mqhdr->mqh_lock);
        }
    }
    long freeindex = mqhdr->mqh_free;
    if (freeindex == 0) {
        err_dump("mq_send: curmsgs = %ld; free = 0", attr->mq_curmsgs);
    }
    struct msg_hdr *msghdr, *nmsghdr, *pmsghdr;
    nmsghdr = (struct msg_hdr *)&mptr[freeindex];
    nmsghdr->msg_prio = prio;
    nmsghdr->msg_len = len;
    memcpy(nmsghdr + 1, ptr, len); // copy message from caller
    mqhdr->mqh_free = nmsghdr->msg_next; // new freelist head

    long index = mqhdr->mqh_head;
    pmsghdr = NULL;
    while (index != 0) {
        msghdr = (struct msg_hdr *)&mptr[index];
        if (prio > msghdr->msg_prio) {
            if (pmsghdr != NULL) {
                pmsghdr->msg_next = freeindex;
            } else {
                mqhdr->mqh_head = freeindex;
            }
            nmsghdr->msg_next = index;
            break;
        }
        index = msghdr->msg_next;
        pmsghdr = msghdr;
    }
    if (index == 0) {
        // queue was empty or new goes at end of list
        if (pmsghdr != NULL) {
            pmsghdr->msg_next = freeindex;
        } else {
            mqhdr->mqh_head = freeindex;
        }
        nmsghdr->msg_next = 0;
    }
    if (attr->mq_curmsgs == 0) {
        pthread_cond_signal(&mqhdr->mqh_wait);
    }
    attr->mq_curmsgs++;
    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return 0;

err:
    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return -1;
}
