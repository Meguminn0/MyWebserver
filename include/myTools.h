#ifndef MYTOOLS_H
#define MYTOOLS_H

#include "timer.h"

// 客户端连接数据，保存客户端的套接字和对应的定时器
struct client_data
{
    int sockfd = -1;
    timer *tm = nullptr;
};

class tools
{
public:
    tools() {}
    ~tools() {}

    // 工具函数
    int setnonblocking(int fd);         /* sockfd设置为非阻塞模式 */
    void epoll_addfd(int fd, bool set_oneShot);     /* 将fd加入epoll监听集合中 */
    void addsig(int sig, void(handler)(int));       /* 增加信号以及对应的处理函数 */
    static void sig_handler(int sig);               /* 信号处理函数 */
    void timer_handler();               /* 处理定时器管理类中的所有超时的定时器 */

    static void timer_callBack(int sockfd);    /* 每个定时器需要执行的任务 */

public:
    static int epollfd;
    static int TIMESLOT;    /* 定时器定时间隔 */
    static int *sockPipefd; /* socketpair 管道 */
    timerManager timer_mg;  /* 管理定时器 */
};

#endif