#ifndef __MYSQL_DATABASE_H__
#define __MYSQL_DATABASE_H__

#include <string>
#include <mysql/mysql.h>

// mysql数据库封装类
class mysql_database
{
public:
    mysql_database(std::string username, std::string pwd, std::string dbName);
    mysql_database(std::string username, std::string pwd, std::string dbName, std::string addr, std::string port);
    ~mysql_database();

    // 数据库连接
    bool connect();
    // 表中字段查询
    bool getTableFeild(std::string tableName);
    // 表查询
    bool show_table(std::string tableName);
    // 自由执行命令
    bool freedom_order(std::string sentence);

    std::string* getResult();

private:
    MYSQL* _mysql;      /* mysql对象 */
    bool _conntStatus;  /* 连接状态 */

    std::string m_addr;           /* 主机地址 */
    std::string m_port;           /* 主机端口号 */
    std::string m_userName;       /* 数据库用户名 */
    std::string m_pwd;            /* 数据库密码 */
    std::string m_databaseName;   /* 使用的数据库名 */
};

#endif