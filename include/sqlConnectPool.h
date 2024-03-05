#ifndef __SQL_CONNECT_POOL_H__
#define __SQL_CONNECT_POOL_H__

#include <list>
#include <string>
#include <mysql/mysql.h>

// SQL连接池，这里使用的是 mySQL
class sqlConnectPool
{
public:
    struct MYSQL* getSqlConnection();              /* 获取池中的SQL连接对象 */
    bool releaseConnection(MYSQL*);         /* 释放SQL连接对象 */
    
    // 初始化SQL连接池
    bool init(int cnt, std::string addr, std::string port, 
                std::string userName, std::string pwd, std::string databaseName);

    // 可以有多个SQL连接池，每个池对应一个数据库，方便分库分表
    static sqlConnectPool* CreateSqlConnectPool();

protected:
    inline sqlConnectPool() { }
    ~sqlConnectPool();

    // 销毁连接池
    void destroyPool();

private:
    sqlConnectPool(const sqlConnectPool&) = delete;
    sqlConnectPool& operator=(const sqlConnectPool&) = delete;


    std::list<MYSQL*> connect_lists;
    
    std::string host_add;           /* 主机地址 */
    std::string host_port;          /* 主机端口号 */
    std::string db_userName;        /* 数据库用户名 */
    std::string db_pwd;             /* 数据库密码 */
    std::string db_BatabaseName;    /* 使用的数据库名 */
};

#endif