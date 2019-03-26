#include "unp.h"
#include "mqueue.h"

int mq_notify(mqd_t mqd, const struct sigevent *notification) {
    struct mq_info *mqinfo = mqd;
    if (mqinfo->mqi_magic != MQI_MAGIC) {
        errno = EBADF;
        return -1;
    }
    struct mq_hdr *mqhdr = mqinfo->mqi_hdr;
    
    int n;
    if ((n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) {
        errno = n;
        return -1;
    }
    pid_t pid = getpid();
    if (notification == NULL) {
        if (mqhdr->mqh_pid == pid) {
            mqhdr->mqh_pid = 0;
        }
    } else {
        if (mqhdr->mqh_pid != 0) {
            // kill(pid, 0) 检测是否有进程注册了信号，如果有则返回EBUSY，否则返回ESRCH
            if (kill(mqhdr->mqh_pid, 0) != -1 || errno != ESRCH) {
                errno = EBUSY;
                goto err;
            }
        }
        mqhdr->mqh_pid = pid;
        mqhdr->mqh_event = *notification;
    }
    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return 0;

err:
    pthread_mutex_unlock(&mqhdr->mqh_lock);
    return -1;
}
