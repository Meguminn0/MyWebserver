#include "http_connect.h"

int http_connect::epollfd = -1;

void http_connect::init(int fd, const struct sockaddr_in &addr, char *root, 
                            std::string sqlUser, std::string sqlPwd, std::string sqlDatabaseName)
{
    m_sockfd = fd;
    m_sockfd_addr = addr;

    addfd(epollfd, m_sockfd, false);

    m_rootPath = root;
    m_sqlUser = sqlUser;
    m_sqlPwd = sqlPwd;
    m_sqlDatabaseName = sqlDatabaseName;

    init();
}

void http_connect::addfd(int epfd, int fd, bool one_shot)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLRDHUP;

    if(one_shot)
    {
        ev.events |= EPOLLONESHOT;
    }

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblock(fd);
}

void http_connect::init()
{
    mysqlDB = nullptr;
    memset(readBuff, 0, READBUFF_MAX_LEN);
    memset(writeBuff, 0, WRITEBUFF_MAX_LEN);
}

int http_connect::setnonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}