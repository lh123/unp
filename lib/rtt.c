#include "unprtt.h"
#include <sys/time.h>

int rtt_d_flag = 0;

/*
 * Calculate the RTO value based on current estimators:
 *      smoothed RTT plus four times the deviation.
 */
#define RTT_RTOCALC(ptr) ((ptr)->rtt_srtt + (4.0 * (ptr)->rtt_rttvar))

static float rtt_minmax(float rto) {
    if (rto < RTT_RXTMIN){
        rto = RTT_RXTMIN;
    } else if (rto > RTT_RXTMAX) {
        rto = RTT_RXTMAX;
    }
    return rto;
}

void rtt_init(struct rtt_info *ptr) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ptr->rtt_base = tv.tv_sec;
    ptr->rtt_rtt = 0;
    ptr->rtt_srtt = 0;
    ptr->rtt_rttvar = 0.75;
    ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr));
    // first RTO at (srtt + (4 * rttvar)) = 3 seconds
}

// 返回当前时间戳
uint32_t rtt_ts(struct rtt_info *ptr) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint32_t ts = ((tv.tv_sec - ptr->rtt_base) * 1000) + (tv.tv_usec / 1000);
    return ts;
}

/* 
 * 把重传计数器设置为0。
 * 每当第一次发送一个新的分组时，都得调用这个函数
 */
void rtt_newpack(struct rtt_info *ptr) {
    ptr->rtt_nrexmt = 0;
}

/*
 * 以秒为单位返回当前RTO。
 * return value can be used as: alarm(rtt_start(&foo))
 */ 
int rtt_start(struct rtt_info *ptr) {
    return (int)(ptr->rtt_rto + 0.5); // round float to int
}

/*
 * 用于更新RTT估算因子并重新计算新的RTO
 */
void rtt_stop(struct rtt_info *ptr, uint32_t ms) {
    ptr->rtt_rtt = ms / 1000.0; // measured RTT in seconds

    /*
     * Update our estimators of RTT and mean deviation of RTT.
     * See Jacobson's SIGCOMM '88 paper, Appendix A, for the details.
     * We use floating point here for simplicity.
     */
    double delta = ptr->rtt_rtt - ptr->rtt_srtt;
    ptr->rtt_srtt += delta / 8;
    if (delta < 0.0) {
        delta = -delta;
    }
    ptr->rtt_rttvar += (delta - ptr->rtt_rttvar) / 4;
    ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr));
}

int rtt_timeout(struct rtt_info *ptr) {
    ptr->rtt_rto *= 2;
    if (++ptr->rtt_nrexmt > RTT_MAXNREXMT) {
        return -1; // time to give up for this packet.
    }
    return 0;
}
