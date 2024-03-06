#include "webserver.h"

WebServer::WebServer()
{

}

WebServer::~WebServer()
{

}
void WebServer::init_web(int webPort)
{
    web_port = webPort;
}

// WebServer 数据库连接初始化
void WebServer::init_sql(int sqlNum, std::string sqlUserName, std::string sqlPwd, std::string databasesName, 
                            std::string sqlAddr, std::string sqlPort)
{
    sqlPool_sqlNum = sqlNum;
    sql_addr = sqlAddr;
    sql_port = sqlPort;
    sql_userName = sqlUserName;
    sql_pwd = sqlPwd;
    sql_databaseName = databasesName;

    sql_pool = sqlConnectPool::CreateSqlConnectPool();
    sql_pool->init(sqlPool_sqlNum, sql_addr, sql_port, sql_userName, sql_pwd, sql_databaseName);
}

void WebServer::init_thread_pool(int threadsNum)
{
    threads_num = threadsNum;
    thd_Pool = new threadPool(threads_num);
}