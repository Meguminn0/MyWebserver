#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/epoll.h>
#include <cstring>
#include <cerrno>

#include <assert.h>
#include <cstring>

#include "my_threadPool.h"
#include "sqlConnectPool.h"
#include "http_connect.h"
#include "timer.h"
#include "myTools.h"
#include "log.h"

#include "debug.h"

#define MAX_EPOLLEVENT_NUM 4096     // 最大epoll事件数
#define MAX_FD 65536             // 最大文件描述符

class webServer
{
public:
    webServer();
    ~webServer();

    // WebServer 基础信息初始化
    void init_web(std::string webIP, std::string webPort);
    //日志初始化
    void init_log(std::string logFilePath, int logBuffSize = 1024, int maxLogLines = 500000);
    // WebServer 数据库连接初始化
    void init_sql(int sqlNum, std::string sqlUserName, std::string sqlPwd, std::string databasesName, 
                    std::string sqlAddr, std::string sqlPort);
    // WebServer 线程池初始化
    void init_thread_pool(int threadsNum);
    // 网络监听
    void webListen();
    // 循环处理监听事件
    void eventLoop();

    static void read_work(http_connect* http);
    static void write_work(http_connect* http);

private:
    // 处理信号
    bool doSignal(bool *stop);
    // 处理客户端连接请求
    bool doClientRequest();
    // 关闭客户端连接
    void closeConnect(int sockfd);
    // 读取客户端数据
    void doClientRead(int sockfd);
    // 向客户端发送数据
    void doClientWrite(int sockfd);

private:
    // webserver相关变量
    std::string web_IP;         /* web服务所在的IP */
    int web_port;               /* web服务所在的端口号 */
    int sockPipefd[2];          /* socketpair 管道 */
    client_data *cdata;         /* 保存每个连接客户的定时器和套接字 */
    tools tool;                 /* 工具类 */

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

    // http
    http_connect *http_user;
    std::string http_root;           /* http网页目录路径 */
};

#endif