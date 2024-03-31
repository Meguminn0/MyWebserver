#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <iostream>
#include <fstream>

#include "debug.h"

#define DEFAULT_VALUE "0"

class config
{
public:
    config();
    ~config() {};

    bool getconfig();

    // web信息
    std::string webIP = "0";                /* web服务器IP，0为默认值，由系统决定 */
    std::string webPort = "2024";          /* web服务器端口, 0为默认值: 2024*/

    // 数据库信息
    std::string sqlHost = "localhost";      /* 数据库地址，0为默认值： localhost */
    std::string sqlPort = "3306";           /* 数据库端口号, 0为默认值： 3306 */
    std::string sqlUserName;                /* 数据库用户名 */
    std::string sqlPwd;                     /* 数据库密码 */
    std::string sqlDatabaseName;            /* 使用的数据库名 */
    
    // 日志信息
    std::string logFilePath;    // 日志文件路径
    int logBuffSize;            // 日志缓冲区大小
    int maxLogLines;            // 日志文件最大行数

    //日志写入方式
    int LOGWrite;
    //触发组合模式
    int TRIGMode;
    //listenfd触发模式
    int LISTENTrigmode;
    //connfd触发模式
    int CONNTrigmode;
    //优雅关闭链接
    int OPT_LINGER;
    //数据库连接池数量
    int sql_num;
    //线程池内的线程数量
    int thread_num;
    //是否关闭日志
    int close_log;
    //并发模型选择
    int actor_model;
};

#endif