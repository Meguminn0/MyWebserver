#ifndef __MYSQL_DATABASE_H__
#define __MYSQL_DATABASE_H__

#include <iostream>
#include <string>
#include <mysql/mysql.h>

#include "debug.h"
// #define DEBUG

// mysql数据库封装类
class mysql_database
{
public:
    mysql_database(std::string username, std::string pwd, std::string dbName) noexcept;
    mysql_database(std::string username, std::string pwd, std::string dbName, std::string host, std::string port) noexcept;
    ~mysql_database();

    // 数据库连接
    bool connect();
    // 查询表中字段数
    long getTableFieldNum(std::string tableName);
    // 表查询
    bool show_table(std::string tableName);
    // 插入数据
    bool add(std::string add_sentence);
    // 任意查询，返回结果集
    MYSQL_RES* inquire(std::string quire_sentence);

protected:
    void show_select();

private:
    MYSQL* _mysql;              /* mysql对象 */
    bool _conntStatus;          /* 连接状态 */
    MYSQL_RES* _res;            /* 查询结果集 */
    MYSQL_ROW _row;             /* 数据行 */

    std::string m_host;           /* 主机地址 */
    std::string m_port;           /* 主机端口号 */
    std::string m_userName;       /* 数据库用户名 */
    std::string m_pwd;            /* 数据库密码 */
    std::string m_databaseName;   /* 使用的数据库名 */
};

#endif