#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "my_threadPool.h"

class WebServer
{
public:
    WebServer();
    ~WebServer();

    void thread_pool();
};

#endif