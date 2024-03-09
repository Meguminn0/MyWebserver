#include "config.h"

config::config()
{
    //日志写入方式，默认同步
    LOGWrite = 0;

    //触发组合模式,默认listenfd LT + connfd LT
    TRIGMode = 0;

    //listenfd触发模式，默认LT
    LISTENTrigmode = 0;

    //connfd触发模式，默认LT
    CONNTrigmode = 0;

    //优雅关闭链接，默认不使用
    OPT_LINGER = 0;

    //关闭日志,默认不关闭
    close_log = 0;

    //并发模型,默认是proactor
    actor_model = 0;
}

bool config::getconfig()
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
                    webPort = "2024";
                }
            }
            else if(str_config.substr(0, idx) == "sqlHost")
            {
                sqlHost = str_config.substr(idx + 1, str_config.size());
                if(sqlHost == DEFAULT_VALUE)
                {
                    sqlPort = "localhost";
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

#ifdef DEBUG
    std::cout << "========== config message ==========\n";
    std::cout << "webIP: " << webIP << '\n';
    std::cout << "webPort: " << webPort << '\n';
    std::cout << "sqlHost: " << sqlHost << '\n';
    std::cout << "sqlPort: " << sqlPort << '\n';
    std::cout << "sqlUserName: " << sqlUserName << '\n';
    std::cout << "sqlPwd: " << sqlPwd << '\n';
    std::cout << "sqlDatabaseName: " << sqlDatabaseName << '\n';
    std::cout << "============ config end ============\n";
#endif

    return true;
}