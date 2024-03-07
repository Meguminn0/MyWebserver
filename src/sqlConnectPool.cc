#include "sqlConnectPool.h"

mysql_database* sqlConnectPool::getConnection()
{
    mysql_database* retDB = nullptr;
    // 加锁
    std::unique_lock<std::mutex> unilock(list_mtx);

    // 当池中 sql 用完时，当前线程先阻塞，等其他线程放弃 sql
    cv.wait(unilock, [this]{ return sql_inPoolNum > 0 || poolClose; });
    if(poolClose)
    {
        unilock.unlock();
        return nullptr;
    }
    retDB = connect_lists.front();
    connect_lists.pop_front();
    --sql_inPoolNum;
    
    unilock.unlock();

    return retDB;
}

bool sqlConnectPool::releaseConnection(mysql_database *mysqlDB)
{
    if(mysqlDB == nullptr)
    {
        return false;
    }

    std::unique_lock<std::mutex> unilock(list_mtx);
    connect_lists.push_back(mysqlDB);
    ++sql_inPoolNum;
    unilock.unlock();
    // 唤醒阻塞中的线程
    cv.notify_one();

    return true;
}

void sqlConnectPool::init(int maxNum, std::string host, std::string port, 
            std::string userName, std::string pwd, std::string dbName)
{
    sql_maxNum = maxNum;
    host = host;
    port = port;
    db_userName = userName;
    db_pwd = pwd;
    db_databaseName = dbName;

    for(int i = 0; i < sql_maxNum; ++i)
    {
        mysql_database *mysqlDB = new mysql_database(db_userName, db_pwd, db_databaseName, host, port);
        connect_lists.push_back(mysqlDB);
        ++sql_inPoolNum;
    }
}


sqlConnectPool* sqlConnectPool::getSqlPool()
{
    static sqlConnectPool sqlPool;
    return &sqlPool;
}

sqlConnectPool::~sqlConnectPool()
{
    destroyPool();
}

void sqlConnectPool::destroyPool()
{
    poolClose = true;
    std::unique_lock<std::mutex> unilock(list_mtx);

    if(connect_lists.size() > 0)
    {
        for(auto item : connect_lists)
        {
            delete item;
        }
        sql_inPoolNum = 0;
        connect_lists.clear();
    }
}