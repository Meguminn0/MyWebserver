#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/epoll.h>

#include <assert.h>
#include <cstring>

#include "my_threadPool.h"
#include "sqlConnectPool.h"

#include "debug.h"

#define MAX_EPOLLEVENT_NUM 4096

class WebServer
{
public:
    inline WebServer() { }
    ~WebServer();

    // WebServer 基础信息初始化
    void init_web(std::string webIP, std::string webPort);

    // WebServer 数据库连接初始化
    void init_sql(int sqlNum, std::string sqlUserName, std::string sqlPwd, std::string databasesName, 
                    std::string sqlAddr, std::string sqlPort);

    // WebServer 线程池初始化
    void init_thread_pool(int threadsNum);

    // 网络监听
    void WebListen();

    // 循环处理监听事件
    void eventLoop();

private:
    // 内部封装的函数
    int setnonblocking(int fd);    /* sockfd设置为非阻塞模式 */
    void epoll_addfd(int fd, bool set_oneShot, int TRIGMode);   /* 将fd加入epoll监听集合中 */

    // webserver相关变量
    std::string web_IP; /* web服务所在的IP */
    int web_port;       /* web服务所在的端口号 */

    // 数据库相关变量
    sqlConnectPool* sql_pool = nullptr; /* 数据库连接池指针 */
    std::string sql_addr;               /* 数据库地址，默认为 localhost */
    std::string sql_port;               /* 数据库端口，默认为 3306 */
    std::string sql_userName;           /* 数据库用户名 */
    std::string sql_pwd;                /* 数据库密码 */
    std::string sql_databaseName;       /* 使用的数据库名 */
    int sqlPool_sqlNum;                 /* sql连接池中的sql对象数量 */

    // 线程池相关变量
    threadPool* thd_Pool = nullptr;     /* 线程池指针 */
    int threads_num;                    /* 线程池中的最大线程数 */

    // 网络IO
    int sockfd_listen = -1;
    int epollfd = -1;
    struct epoll_event epollEvents[MAX_EPOLLEVENT_NUM];
};

#endif