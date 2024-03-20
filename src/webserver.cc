#include "webserver.h"

webServer::webServer()
{
    http_user = new http_connect[MAX_FD];
    cdata = new client_data[MAX_FD];
    http_root = "./Root";
}

webServer::~webServer()
{
    close(tools::sockPipefd[0]);
    close(tools::sockPipefd[1]);
    
    delete thd_Pool;
    delete[] http_user;
    delete[] cdata;
}

void webServer::init_web(std::string webIP, std::string webPort)
{
    web_IP = webIP;
    web_port = std::stoi(webPort);
}

// webServer 数据库连接初始化
void webServer::init_sql(int sqlNum, std::string sqlUserName, std::string sqlPwd, std::string databasesName, 
                            std::string sqlAddr, std::string sqlPort)
{
    sqlPool_sqlNum = sqlNum;
    sql_addr = sqlAddr;
    sql_port = sqlPort;
    sql_userName = sqlUserName;
    sql_pwd = sqlPwd;
    sql_databaseName = databasesName;

    sql_pool = sqlConnectPool::getSqlPool();
    sql_pool->init(sqlPool_sqlNum, sql_addr, sql_port, sql_userName, sql_pwd, sql_databaseName);
}

void webServer::init_thread_pool(int threadsNum)
{
    threads_num = threadsNum;
    thd_Pool = new threadPool(threads_num);
}

void webServer::webListen()
{
    int flag = 0;
    sockfd_listen = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd_listen == -1)
    {
#ifdef DEBUG
        printf("(%s %s) %s:%s(%ld) %s\n", __DATE__, __TIME__, 
                __FILE__, __func__, __LINE__, "Create sockfd error!");
#endif
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr_server;
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = inet_addr(web_IP.c_str());
    addr_server.sin_port = htons(web_port);

    // 设置端口复用
    int optval = 1;
    setsockopt(sockfd_listen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    flag = bind(sockfd_listen, (struct sockaddr*)&addr_server, sizeof(addr_server));
    if(flag == -1)
    {
#ifdef DEBUG
        printf("(%s %s) %s:%s(%ld) %s\n", __DATE__, __TIME__, 
                __FILE__, __func__, __LINE__, "bind error!");
#endif
        exit(EXIT_FAILURE);
    }

    flag = listen(sockfd_listen, 10);
    if(flag == -1)
    {
#ifdef DEBUG
        printf("(%s %s) %s:%s(%ld) %s\n", __DATE__, __TIME__, 
                __FILE__, __func__, __LINE__, "listen error!");
#endif
        exit(EXIT_FAILURE);
    }

    // 使用 epoll IO多路复用模型
    epollfd = epoll_create(1024);
    if(epollfd == -1)
    {
#ifdef DEBUG
        printf("(%s %s) %s:%s(%ld) %s\n", __DATE__, __TIME__, 
                __FILE__, __func__, __LINE__, "Create epoll error!");
#endif
        exit(EXIT_FAILURE);
    }
    // 同时给http_connect类和tools类中的epollfd赋值
    http_connect::epollfd = epollfd;
    tools::epollfd = epollfd;
    tool.epoll_addfd(sockfd_listen, false);

    // 创建socketpair本地本地套接字对，用于传输信号
    // 虽然它一般用于进程间管导通信，为了让项目更有逼格，多学一点
    int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, sockPipefd);
    tools::sockPipefd = sockPipefd;
    assert(ret != -1);
    tool.setnonblocking(sockPipefd[1]);
    tool.epoll_addfd(sockPipefd[0], false);
    
    tool.addsig(SIGPIPE, SIG_IGN);
    tool.addsig(SIGALRM, tool.sig_handler);
    tool.addsig(SIGTERM, tool.sig_handler);
    // 使用alarm设置定时周期
    alarm(5);
}

void webServer::eventLoop()
{
    bool stop_server = false;

    while(!stop_server)
    {
        int eventNum = epoll_wait(epollfd, epollEvents, MAX_EPOLLEVENT_NUM, -1);
        if(eventNum < 0 && errno != EINTR)
        {
#ifdef DEBUG
            printf("(%s %s) %s:%s(%ld) %s\n", __DATE__, __TIME__, 
                __FILE__, __func__, __LINE__, "epoll error!");
#endif
            break;
        }
        
        for(int i = 0; i < eventNum; ++i)
        {
            int sockfd = epollEvents[i].data.fd;
            if(sockfd == sockfd_listen)
            {
#ifdef DEBUG
               printf("客户端已连接\n");
#endif
                // 处理客户端连接请求
                bool flag = doClientRequest();
                if(flag == false)
                {
                    continue;
                }
            }
            else if(epollEvents[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 如果对方 读关闭|挂断连接|出现异常，关闭连接
                closeConnect(sockfd);
            }
            else if ((sockfd == tool.sockPipefd[0]) && (epollEvents[i].events & EPOLLIN))
            {
                // 收到定时信号
                doSignal(&stop_server);
            }
            
            else if(epollEvents[i].events & EPOLLIN)
            {
                // 处理客户端发送的数据
                doClientRead(sockfd);
            }
            else if(epollEvents[i].events & EPOLLOUT)
            {
                // 向客户端发送数据
                doClientWrite(sockfd);
            }
        }
    }
}


bool webServer::doSignal(bool *stop)
{
    char signals[1024];
    int ret = recv(tool.sockPipefd[0], signals, sizeof(signals), 0);
    if(ret <= 0)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < ret; ++i)
        {
            if (signals[i] == SIGALRM)
            {
                tool.timer_handler();
                break;
            }
            else if(signals[i] == SIGTERM)
            {
                *stop = true;
                break;
            }
        }
    }

    return true;
}

bool webServer::doClientRequest()
{
    struct sockaddr_in client_addr;
    socklen_t client_addrLen = sizeof(client_addr);
    int sockfd_client = accept(sockfd_listen, (struct sockaddr*)&client_addr, &client_addrLen);
    if(sockfd_client == -1)
    {
#ifdef DEBUG
        printf("(%s %s) %s:%s(%ld) %s\n", __DATE__, __TIME__, 
                __FILE__, __func__, __LINE__, "accept error!");
#endif
        return false;
    }
    
    http_user[sockfd_client].init(sockfd_client, client_addr, sql_pool, http_root, sql_userName, sql_pwd, sql_databaseName);
    cdata[sockfd_client].sockfd = sockfd_client;
    cdata[sockfd_client].tm = new timer(tools::TIMESLOT * 3);
    cdata[sockfd_client].tm->callBack_func(tools::timer_callBack, sockfd_client);
    tool.timer_mg.add_timer(cdata[sockfd_client].tm);

    return true;
}

void webServer::closeConnect(int sockfd)
{
#ifdef DEBUG
    printf("(%s %s) %s:%s(%ld) %s\n", __DATE__, __TIME__, 
                __FILE__, __func__, __LINE__, "close");
#endif
    timer *timer = cdata[sockfd].tm;
    timer->action_func();
    if(timer)
    {
        tool.timer_mg.delete_timer(timer);
        cdata[sockfd].tm = nullptr;
    }
}

void webServer::doClientRead(int sockfd)
{
    // 处理客户端发来的数据
    // 将客户端对应的http对象的数据处理状态设为读取状态
    http_user[sockfd].m_RWStat = http_connect::HTTP_READ_STATUS;
    // 将套接字对应的定时器时间向后延迟
    timer* tm = cdata[sockfd].tm;
    if(tm)
    {
        tm->setExpireTIme(tm->getNow_sec() + tools::TIMESLOT * 3);
        tool.timer_mg.adjust_timer(tm);
    }
    // 将读取任务放入线程池中执行
    thd_Pool->append_task(read_work, &http_user[sockfd]);
    while(true)
    {
        if(http_user[sockfd].m_RWflag)
        {
            if(http_user[sockfd].m_RWErrorFlag)
            {
                // 读取错误，删除对应的定时器，并关闭连接
                closeConnect(sockfd);
                http_user[sockfd].m_RWErrorFlag = false;
            }
            http_user[sockfd].m_RWflag = false;
            break;
        }
    }
}

void webServer::doClientWrite(int sockfd)
{
    // 将准备好的数据发送给客户端
    // 将客户端对应的http对象的数据处理状态设为写入状态
    http_user[sockfd].m_RWStat = http_connect::HTTP_WRITE_STATUS;
    // 将套接字对应的定时器时间向后延迟
    timer* tm = cdata[sockfd].tm;
    if(tm)
    {
        tm->setExpireTIme(tm->getNow_sec() + tools::TIMESLOT * 3);
        tool.timer_mg.adjust_timer(tm);
    }
    // 将写入任务放入线程池中执行
    thd_Pool->append_task(write_work, &http_user[sockfd]);
    while(true)
    {
        if(http_user[sockfd].m_RWflag)
        {
            if(http_user[sockfd].m_RWErrorFlag)
            {
                // 读取错误，删除对应的定时器，并关闭连接
                closeConnect(sockfd);
                http_user[sockfd].m_RWErrorFlag = false;
            }
            http_user[sockfd].m_RWflag = false;
            break;
        }
    }
}

void webServer::read_work(http_connect* http)
{
    if(http->read_once())
    {
        // 读取成功，设置状态并开始解析数据
        http->m_RWflag = true;
        http->read();
    }
    else
    {
        // 读取失败，设置异常状态
        http->m_RWflag = true;
        http->m_RWErrorFlag = true;
    }
}

void webServer::write_work(http_connect* http)
{
    if(http->write())
    {
        // 发送完成，设置状态
        http->m_RWflag = true;
    }
    else
    {
        // 发送失败，设置异常状态
        http->m_RWflag = true;
        http->m_RWErrorFlag = true;
    }
}
