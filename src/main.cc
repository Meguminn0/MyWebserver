#include "config.h"
#include "webserver.h"

int main()
{
    config cfg;
    // 从配置文件读取需要的数据
    if(!cfg.getconfig())
    {
        return EXIT_FAILURE;
    }

    webServer webserver;

    // 服务器初始化
    webserver.init_web(cfg.webIP, cfg.webPort);

    // 日志初始化
    webserver.init_log(cfg.logFilePath, cfg.logBuffSize, cfg.maxLogLines);

    // 数据库初始化
    webserver.init_sql(10, cfg.sqlUserName, cfg.sqlPwd, 
                cfg.sqlDatabaseName, cfg.sqlHost, cfg.sqlPort);
    
    // 线程池初始化
    webserver.init_thread_pool(6);
    // 打开监听
    webserver.webListen();
    
    // 运行
    webserver.eventLoop();

    return 0;
}
