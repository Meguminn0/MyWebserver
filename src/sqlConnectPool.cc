#include "sqlConnectPool.h"

struct MYSQL* sqlConnectPool::getSqlConnection()
{
    return nullptr;
}

bool sqlConnectPool::releaseConnection(MYSQL *mysql)
{
    return true;
}

bool sqlConnectPool::init(int num, std::string addr, std::string port, 
            std::string userName, std::string pwd, std::string dbName)
{
    sql_num = num;
    host_addr = addr;
    host_port = port;
    db_userName = userName;
    db_pwd = pwd;
    db_databaseName = dbName;

    for(int i = 0; i < sql_num; ++i)
    {

    }

    return true;
}


sqlConnectPool* sqlConnectPool::CreateSqlConnectPool()
{
    sqlConnectPool *sqlPool = new sqlConnectPool();
    return sqlPool;
}

sqlConnectPool::~sqlConnectPool()
{

}

void sqlConnectPool::destroyPool()
{

}