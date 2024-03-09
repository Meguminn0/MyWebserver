#ifndef __HTTP_CONNECT_H__
#define __HTTP_CONNECT_H__

#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mysql_database.h"

class http_connect
{
public:
    static const int READBUFF_MAX_LEN = 2048;
    static const int WRITEBUFF_MAX_LEN = 1024;

    inline http_connect() {}
    inline ~http_connect() {}

    /* 初始化http的网络连接和数据库信息
     fd: 套接字
     addr: 套接字对应的地址
     root: http 网页文件目录
     sqlUsr: sql用户名
     sqlPwd: sql密码
     sqlDatabaseName: sql数据库名
    */
    void init(int fd, const struct sockaddr_in &addr, char *root, 
                std::string sqlUser, std::string sqlPwd, std::string sqlDatabaseName);

    // 将fd加入到epoll集合中
    void addfd(int epfd, int fd, bool one_shot);
    // 初始化http相关信息
    void init();

    //对文件描述符设置非阻塞
    int setnonblock(int fd);
    static int epollfd;

private:
    int m_sockfd;
    struct sockaddr_in m_sockfd_addr;
    char readBuff[READBUFF_MAX_LEN];
    char writeBuff[WRITEBUFF_MAX_LEN];

    std::string m_rootPath;

    mysql_database *mysqlDB;
    std::string m_sqlUser;
    std::string m_sqlPwd;
    std::string m_sqlDatabaseName;
};

#endif