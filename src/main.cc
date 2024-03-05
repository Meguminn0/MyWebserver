#include <iostream>
#include <fstream>

#include "webserver.h"
#include <string>

#define DEBUG

// 数据库信息
std::string user_name;      /* 数据库用户名 */
std::string pwd;            /* 数据库密码 */
std::string databaseName;   /* 使用的数据库名 */

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
            if(str_config.substr(0, idx) == "userName")
            {
                user_name = str_config.substr(idx + 1, str_config.size());
            }
            else if(str_config.substr(0, idx) == "pwd")
            {
                pwd = str_config.substr(idx + 1, str_config.size());
            }
            else if(str_config.substr(0, idx) == "databaseName")
            {
                databaseName = str_config.substr(idx + 1, str_config.size());
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
    std::cout << "== config message ==\n";
    std::cout << "user_name: " << user_name << '\n';
    std::cout << "pwd: " << pwd << '\n';
    std::cout << "databaseName: " << databaseName << '\n';
    std::cout << "== config end ==\n";
#endif

    WebServer webserver;

    return 0;
}
