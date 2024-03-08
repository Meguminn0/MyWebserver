#ifndef __SQL_CONNECT_POOL_H__
#define __SQL_CONNECT_POOL_H__

#include <list>
#include <string>
#include <mutex>
#include <condition_variable>
#include <mysql/mysql.h>

#include "mysql_database.h"
#include "debug.h"

// SQL连接池，这里使用的是 mySQL
class sqlConnectPool
{
public:
    mysql_database* getConnection();            /* 获取池中的SQL连接对象 */
    bool releaseConnection(mysql_database*);    /* 释放SQL连接对象 */
    
    // 初始化SQL连接池
    void init(int num, std::string addr, std::string port, 
                std::string userName, std::string pwd, std::string dbName);

    // 销毁连接池
    void destroyPool();
    
    // 单例模式
    static sqlConnectPool* getSqlPool();

    ~sqlConnectPool();
private:
    inline sqlConnectPool() : sql_inPoolNum(0), poolClose(false) { }
    sqlConnectPool(const sqlConnectPool&) = delete;
    sqlConnectPool& operator=(const sqlConnectPool&) = delete;


    std::list<mysql_database*> connect_lists;
    
    std::string host;               /* 主机地址 */
    std::string port;               /* 主机端口号 */
    std::string db_userName;        /* 数据库用户名 */
    std::string db_pwd;             /* 数据库密码 */
    std::string db_databaseName;    /* 使用的数据库名 */
    int sql_maxNum;                 /* sql连接池中的sql对象最大数量 */
    int sql_inPoolNum;              /* 在 sql 连接池中的sql数量 */

    std::mutex list_mtx;            /* sql连接池互斥量 */
    std::condition_variable cv;
    bool poolClose;
};

#endif