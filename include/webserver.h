#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "my_threadPool.h"
#include "sqlConnectPool.h"

class WebServer
{
public:
    WebServer();
    ~WebServer();

    // WebServer 基础信息初始化
    void init_web(int webPort);

    // WebServer 数据库连接初始化
    void init_sql(int sqlNum, std::string sqlUserName, std::string sqlPwd, std::string databasesName, 
                    std::string sqlAddr, std::string sqlPort);

    void init_thread_pool(int threadsNum);

private:
    // webserver相关变量
    int web_port;       /* web服务所在的端口号 */

    // 数据库相关变量
    sqlConnectPool* sql_pool;           /* 数据库连接池指针 */
    std::string sql_addr;               /* 数据库地址，默认为 localhost */
    std::string sql_port;               /* 数据库端口，默认为 3306 */
    std::string sql_userName;           /* 数据库用户名 */
    std::string sql_pwd;                /* 数据库密码 */
    std::string sql_databaseName;       /* 使用的数据库名 */
    int sqlPool_sqlNum;                 /* sql连接池中的sql对象数量 */

    // 线程池相关
    threadPool* thd_Pool;               /* 线程池指针 */
    int threads_num;                    /* 线程池中的最大线程数 */
};

#endif