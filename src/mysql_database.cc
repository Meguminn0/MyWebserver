#include "mysql_database.h"

mysql_database::mysql_database(std::string username, std::string pwd, std::string dbName)
{
    _conntStatus = false;
    m_userName = username;
    m_pwd = pwd;
    m_databaseName = dbName;
}

mysql_database::mysql_database(std::string username, std::string pwd, std::string dbName, 
                                std::string addr, std::string port)
{
    m_addr = addr;
    m_port = port;
    mysql_database(username, pwd, dbName);
}

mysql_database::~mysql_database()
{

}

bool mysql_database::connect()
{
    if(_conntStatus == true)
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s: %s", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "DataBase Connected!");
#endif
        return false;
    }
    return true;
}

bool mysql_database::getTableFeild(std::string tableName)
{
    return true;
}

bool mysql_database::show_table(std::string tableName)
{
    return true;
}

bool mysql_database::freedom_order(std::string sentence)
{
    return true;
}

std::string* mysql_database::getResult()
{
    return nullptr;
}