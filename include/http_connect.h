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
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/uio.h>

#include "sqlConnectPool.h"

class http_connect
{
public:
    static const int FILE_LENTH = 1024;
    static const int HTTP_READ_STATUS = 0;      /* http数据读取状态 */
    static const int HTTP_WRITE_STATUS = 1;     /* http数据读取状态 */
    static const int READBUFF_MAX_LEN = 2048;   /* 读取数据的最大长度 */
    static const int WRITEBUFF_MAX_LEN = 1024;  /* 写数据的最大长度 */


    //定义http响应的一些状态信息
    const char *ok_200_title = "OK";
    const char *error_400_title = "Bad Request";
    const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
    const char *error_403_title = "Forbidden";
    const char *error_403_form = "You do not have permission to get file form this server.\n";
    const char *error_404_title = "Not Found";
    const char *error_404_form = "The requested file was not found on this server.\n";
    const char *error_500_title = "Internal Error";
    const char *error_500_form = "There was an unusual problem serving the request file.\n";

    /* 
     请求方法
     http的所有请求方法
    */
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };

    /* 
     执行解析动作，有三种，分别是：
     CHECK_STATE_REQUESTLINE    解析http协议的请求行
     CHECK_STATE_HEADER         解析http协议的头部
     CHECK_STATE_CONTENT        解析http协议的内容 
    */
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    /* 
     http状态代码
     NO_REQUEST             无请求
     GET_REQUEST            获得请求
     BAD_REQUEST            错误的请求
     NO_RESOURCE            请求文件不存在       
     FORBIDDEN_REQUEST      请求文件不可读
     FILE_REQUEST           请求文件准备就绪
     INTERNAL_ERROR         内部错误
     CLOSED_CONNECTION      
    */
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    /* 
     行的读取状态
    */
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    inline http_connect() {}
    inline ~http_connect() {}

    /* 初始化http的网络连接和数据库信息
     fd: 套接字
     addr: 套接字对应的地址
     sqlPool: sql数据库连接池指针
     root: http 网页文件目录
     sqlUsr: sql用户名
     sqlPwd: sql密码
     sqlDatabaseName: sql数据库名
    */
    void init(int fd, const struct sockaddr_in &addr, sqlConnectPool *sqlcnnPool, std::string root, 
                            std::string sqlUser, std::string sqlPwd, std::string sqlDatabaseName);

    // 将fd加入到epoll集合中
    void addfd(int epfd, int fd, bool one_shot);
    // 初始化http相关信息
    void init();
    //对文件描述符设置非阻塞
    int setnonblock(int fd);

    // 读取数据的函数入口
    void read();
    // 写入数据的函数入口
    void write();

private:
    //将事件重置为EPOLLONESHOT
    void modfd(int epollfd, int fd, int ev, int TRIGMode);
    // 读取一次客户端发来的数据，可能不会一次性读取完
    bool read_once();

    // 解析读取的内容
    HTTP_CODE read_parse();
    // 解析一行数据是否完整，并用字符'\0\0'区分每行数据
    LINE_STATUS line_end_parse();
    // 解析请求行
    HTTP_CODE parse_request_line(char *lineBegin);
    // 解析请求头部
    HTTP_CODE parse_headers(char *lineBegin);
    // 解析请求数据
    HTTP_CODE parse_content(char *lineBegin);

    // 做出请求响应
    HTTP_CODE do_request();
    // 将html文件在内存中的映射回收
    void unmap();

    // 解析要写入发送的数据
    bool write_parse(HTTP_CODE readRet);
    // 关闭连接
    void close_conn(bool real_close = true);
    // 将http响应报文的状态行写入write缓冲区
    bool add_status_line(int status, const char *title);
    // 将http响应报文的响应头部写入write缓冲区
    bool add_headers(int content_len);
    // 将http响应报文中响应头部的Content-Length字段写入write缓冲区
    bool add_content_lengthVal(int content_len);
    // 将http响应报文中响应头部的Connection字段写入write缓冲区
    bool add_connectionVal();
    // 将http响应头部与响应正文之间的分割行写入write缓冲区
    bool add_blank_line();
    // 将http响应响应正文写入write缓冲区
    bool add_content(const char *content);
    // 写入write缓冲区方法
    bool add_writeBuff(const char* format, ...);

public:
    static int epollfd;
    int m_RWStat;   /* http数据处理状态：HTTP_READ_STATUS为读，HTTP_WRITE_STATUS为写 */

private:
    // http 基础变量
    int m_sockfd;
    struct sockaddr_in m_sockfd_addr;

    char readBuff[READBUFF_MAX_LEN];    /* read数据缓冲区 */
    long long m_read_idx;               /* read数据缓冲区的最后一个数据的位置 */
    CHECK_STATE m_check_state;          /* 当前执行解析的动作 */
    long long m_check_idx;              /* 解析内容的read数据缓冲区位置 */
    long long m_line_idx;               /* 解析数据时，每行在read数据缓冲区的起始位置位置 */

    METHOD m_currentMethod;             /* 当前http的请求方法 */
    char* m_currentURL;                 /* 当前http请求的 URL */
    bool m_head_connectionVal;          /* 请求头部的connection字段值，值为keep-alive时为 true,否则为false */
    char* m_head_hostVal;               /* 请求头部的host字段值 */
    char* m_content_text;               /* 当前http请求的请求数据内容 */
    long long m_content_len;            /* 当前http请求的请求数据长度 */

    std::string m_requestFile;          /* 返回给客户端的html页面文件 */
    struct stat m_file_stat;            /* 返回给客户端的html页面文件状态 */
    char *m_file_address;               /* 返回给客户但的html页面文件在内存中的映射地址 */
    struct iovec m_iov[2];              /* 定义两个io向量元素，用于指向我们需要的资源，如 writebuff 和 需要发送的html文件在内存中的映射地址*/
    int iovLen;                         /* iovec元素的个数 */    

    char writeBuff[WRITEBUFF_MAX_LEN];  /* write数据缓冲区 */
    long long m_write_idx;              /* write数据缓冲区的最后一个数据的位置 */
    long long need_to_sent;             /* 需要向write数缓冲区写入的数据长度 */
    long long has_been_sent;            /* 已经向write数缓冲区写入的数据长度 */


    std::string m_rootPath;             /* http网页文件目录 */

    // sal连接变量
    sqlConnectPool *sql_pool;
    mysql_database *mysqlDB;
    std::string m_sqlUser;
    std::string m_sqlPwd;
    std::string m_sqlDatabaseName;
};

#endif