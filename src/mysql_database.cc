#include "mysql_database.h"

mysql_database::mysql_database(std::string username, std::string pwd, 
                    std::string dbName) noexcept : m_host("localhost"), m_port("3306")
{
    m_userName = username;
    m_pwd = pwd;
    m_databaseName = dbName;
    _conntStatus = false;
    _mysql = nullptr;
    _res = nullptr;
    _row = nullptr;
}

mysql_database::mysql_database(std::string username, std::string pwd, 
                std::string dbName,std::string host, std::string port) noexcept :
            mysql_database(username, pwd, dbName)
{
    m_host = host;
    m_port = port;
}

mysql_database::~mysql_database()
{
    if(_res)
    {
        mysql_free_result(_res);
    }

    if(_conntStatus)
    {
        mysql_close(_mysql);
    }
}

bool mysql_database::connect()
{
    if(_conntStatus == true)
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s\n", __DATE__, __TIME__, __FILE__, __func__, __LINE__,
            "DataBase connected!");
#endif
        return false;
    }

#ifdef DEBUG
    std::cout << "_conntStatus: " << _conntStatus << std::endl;
    std::cout << "m_host: " << m_host << std::endl;
    std::cout << "m_port: " << m_port << std::endl;
    std::cout << "m_userName: " << m_userName << std::endl;
    std::cout << "m_pwd: " << m_pwd << std::endl;
    std::cout << "m_databaseName: " << m_databaseName << std::endl;
#endif

    // 初始化 _mysql
    _mysql = mysql_init(_mysql);
    if(!mysql_real_connect(_mysql, m_host.c_str(), m_userName.c_str(), m_pwd.c_str(),
                            m_databaseName.c_str(), std::stoi(m_port), nullptr, 0))
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "error connect to database:", mysql_error(_mysql));
#endif
        return false;
    }
    else
    {
        _conntStatus = true;
    }

    return true;
}

long mysql_database::getTableFieldNum(std::string tableName)
{
    if(_conntStatus = false)
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "DataBase not connected!");
#endif
        return -1;
    }

    // 生成表结构查询语句
    std::string query = "desc " + tableName;
    // 查询 0成功，1失败
    if(mysql_query(_mysql, query.c_str()))
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "mysql query failed:", mysql_error(_mysql));
#endif
        return -1;
    }
    // 获取查询的结果集
    if(!(_res = mysql_store_result(_mysql)))
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "mysql get result error:", mysql_error(_mysql));
        return -1;
#endif
    }

    // 表结构中每一行代表一列
    return mysql_affected_rows(_mysql);
}

bool mysql_database::show_table(std::string tableName)
{    
    if(_conntStatus = false)
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "DataBase not connected!");
#endif
        return false;
    }

    // 查询表中字段数
    int fieledNum = getTableFieldNum(tableName);
    if(fieledNum == -1)
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "get table field failed");
#endif
        return false;
    }
    
    // 生成表查询语句
    std::string query = "select * from " + tableName;
    // 查询 0成功，1失败
    if(mysql_query(_mysql, query.c_str()))
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "mysql query failed:", mysql_error(_mysql));
#endif
        return false;
    }

    // 获取查询的结果集
    if(!(_res = mysql_store_result(_mysql)))
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "mysql get result error:", mysql_error(_mysql));
#endif
        return false;
    }
    
    // 输出表中字段
    for(int i = 0; i < fieledNum; ++i)
    {
        printf("%-5s\t", mysql_fetch_field(_res)->name);
    }

    puts("");
    // 输出表中每一行
    while(_row = mysql_fetch_row(_res))
    {
        for(int i = 0; i < fieledNum; ++i)
        {
            printf("%-10s\t", _row[i]);
        }
        puts("");
    }
    
    return true;
}

bool mysql_database::add(std::string add_sentence)
{
    if(_conntStatus = false)
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "DataBase not connected!");
#endif
        return false;
    }

    if(mysql_query(_mysql, add_sentence.c_str()))
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "mysql query failed:", mysql_error(_mysql));
#endif
        return false;
    }
    
    return true;
}

MYSQL_RES* mysql_database::inquire(std::string quire_sentence)
{
    if(_conntStatus = false)
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "DataBase not connected!");
#endif
        return nullptr;
    }

    if(mysql_query(_mysql, quire_sentence.c_str()))
    {
#ifdef DEBUG
        printf("[%s %s] %s:%s:(%ld) %s %s\n", __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__, 
                    "mysql query failed:", mysql_error(_mysql));
#endif
        return nullptr;
    }

    return mysql_store_result(_mysql);
}