#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "my_threadPool.h"

class WebServer
{
public:
    WebServer();
    ~WebServer();

    // WebServer 初始化
    void init();

    void thread_pool();
private:
    
};

#endif