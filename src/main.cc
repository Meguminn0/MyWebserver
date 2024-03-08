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

    WebServer webserver;

    // 服务器初始化
    webserver.init_web(cfg.webIP, cfg.webPort);
    
    // 数据库初始化
    webserver.init_sql(10, cfg.sqlUserName, cfg.sqlPwd, 
                cfg.sqlDatabaseName, cfg.sqlHost, cfg.sqlPort);
    
    // 线程池初始化
    webserver.init_thread_pool(8);

    return 0;
}
