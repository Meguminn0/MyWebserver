#include <iostream>
#include <fstream>

#include "webserver.h"
#include <string>

#define DEBUG
#define DEFAULT_VALUE "0"


// web信息
std::string webIP = "0";                /* web服务器IP，0为默认值，由系统决定 */
std::string webPort = "20024";          /* web服务器端口, 0为默认值: 20024*/

// 数据库信息
std::string sqlAddr = "localhost";      /* 数据库地址，0为默认值： localhost */
std::string sqlPort = "3306";           /* 数据库端口号, 0为默认值： 3306 */
std::string sqlUserName;                /* 数据库用户名 */
std::string sqlPwd;                     /* 数据库密码 */
std::string sqlDatabaseName;            /* 使用的数据库名 */

bool getconfig()
{    
    std::ifstream ifs;
    ifs.open("config.ini", std::ios::in);

    if (!ifs.is_open())
    {
        std::cout << "open error\n";
        return false;
    }

    std::string str_config;
    while(ifs >> str_config)
    {
        int idx = str_config.find_first_of('=');
        if(idx != -1)
        {
            if(str_config.substr(0, idx) == "webIp")
            {
                webIP = str_config.substr(idx + 1, str_config.size());
                if(webIP == "0")
                {
                    webIP = "INADDR_ANY";
                }
            }
            else if(str_config.substr(0, idx) == "webPort")
            {
                webPort = str_config.substr(idx + 1, str_config.size());
                if(webPort == DEFAULT_VALUE)
                {
                    webPort = "20024";
                }
            }
            else if(str_config.substr(0, idx) == "sqlPort")
            {
                sqlPort = str_config.substr(idx + 1, str_config.size());
                if(sqlPort == DEFAULT_VALUE)
                {
                    sqlPort = "3306";
                }
            }
            else if(str_config.substr(0, idx) == "sqlPort")
            {
                sqlPort = str_config.substr(idx + 1, str_config.size());
            }
            else if(str_config.substr(0, idx) == "sqlUserName")
            {
                sqlUserName = str_config.substr(idx + 1, str_config.size());
            }
            else if(str_config.substr(0, idx) == "sqlPassword")
            {
                sqlPwd = str_config.substr(idx + 1, str_config.size());
            }
            else if(str_config.substr(0, idx) == "sqlDatabaseName")
            {
                sqlDatabaseName = str_config.substr(idx + 1, str_config.size());
            }
        }
    }

    return true;
}

int main()
{
    // 从配置文件读取需要的数据
    if(!getconfig())
    {
        return EXIT_FAILURE;
    }

#ifdef DEBUG
    std::cout << "========== config message ==========\n";
    std::cout << "webIP: " << webIP << '\n';
    std::cout << "webPort: " << webPort << '\n';
    std::cout << "sqlAddr: " << sqlAddr << '\n';
    std::cout << "sqlPort: " << sqlPort << '\n';
    std::cout << "sqlUserName: " << sqlUserName << '\n';
    std::cout << "sqlPwd: " << sqlPwd << '\n';
    std::cout << "sqlDatabaseName: " << sqlDatabaseName << '\n';
    std::cout << "============ config end ============\n";
#endif

    WebServer webserver;

    webserver.init_web(std::stoi(webPort));
    webserver.init_sql(10, sqlUserName, sqlPwd, sqlDatabaseName, sqlAddr, sqlPort);

    return 0;
}
