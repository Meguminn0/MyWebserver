#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <cstring>
#include <cerrno>
#include <signal.h>
#include <assert.h>
#include <unistd.h>

#include "myTools.h"

int tools::epollfd = -1;
int tools::TIMESLOT = 5;
int *tools::sockPipefd = nullptr;

int tools::setnonblocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);

    return old_opt;
}

void tools::epoll_addfd(int fd, bool set_oneShot)
{
    struct epoll_event ev;
    ev.data.fd = fd;

    ev.events = EPOLLIN;
    if(set_oneShot)
    {
        // 设置之后只监听一次，若还想继续监听，可再将sockfd加入到epoll集合中
        // 服务器监听sockfd不设置，客户端sockdf设置，减少资源消耗，
        // web服务器只有少部分客户端活跃
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);

    // 设置为非阻塞，当sockfd中没有数据时不会被阻塞
    setnonblocking(fd);
}


void tools::addsig(int sig, void(handler)(int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void tools::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(sockPipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void tools::timer_handler()
{
    timer_mg.tick();
    alarm(TIMESLOT);
}

void tools::timer_callBack(int sockfd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, 0);
    close(sockfd);
}