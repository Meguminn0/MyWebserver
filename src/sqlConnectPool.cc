#include "sqlConnectPool.h"

struct MYSQL* sqlConnectPool::getSqlConnection()
{
    MYSQL *mysql = nullptr;
    return mysql;
}

bool sqlConnectPool::releaseConnection(MYSQL *mysql)
{
    return true;
}

bool sqlConnectPool::init(int cnt, std::string addr, std::string port, 
            std::string userName, std::string pwd, std::string databaseName)
{
    return true;
}


sqlConnectPool* sqlConnectPool::CreateSqlConnectPool()
{
    return nullptr;
}

sqlConnectPool::~sqlConnectPool()
{

}

void sqlConnectPool::destroyPool()
{

}