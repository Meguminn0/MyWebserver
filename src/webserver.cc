#include "webserver.h"

WebServer::WebServer()
{
    http_user = new http_connect[MAX_FD];
    http_root = "./Root";
}

WebServer::~WebServer()
{
    delete thd_Pool;
}

void WebServer::init_web(std::string webIP, std::string webPort)
{
    web_IP = webIP;
    web_port = std::stoi(webPort);
}

// WebServer 数据库连接初始化
void WebServer::init_sql(int sqlNum, std::string sqlUserName, std::string sqlPwd, std::string databasesName, 
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

void WebServer::init_thread_pool(int threadsNum)
{
    threads_num = threadsNum;
    thd_Pool = new threadPool(threads_num);
}

void WebServer::WebListen()
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

    // 同时给http_connect类中的epollfd赋值
    http_connect::epollfd = epollfd;
    if(epollfd == -1)
    {
#ifdef DEBUG
        printf("(%s %s) %s:%s(%ld) %s\n", __DATE__, __TIME__, 
                __FILE__, __func__, __LINE__, "Create epoll error!");
#endif
        exit(EXIT_FAILURE);
    }
    
    epoll_addfd(sockfd_listen, false, 0);
}

int WebServer::setnonblocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);

    return old_opt;
}

void WebServer::epoll_addfd(int fd, bool set_oneShot, int TRIGMode)
{
    struct epoll_event ev;
    ev.data.fd = fd;

    ev.events = EPOLLIN;
    if(set_oneShot)
    {
        // 设置之后只监听一次，若还想继续监听，可再将sockfd加入到epoll集合中
        // 服务器监听sockfd不设置，客户端sockdf设置，减少资源消耗，
        // web服务器只有少部分客户端活跃
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);

    // 设置为非阻塞，当sockfd中没有数据时不会被阻塞
    setnonblocking(fd);
}

void WebServer::eventLoop()
{
    bool stop_server = false;

    while(!stop_server)
    {
        int eventNum = epoll_wait(epollfd, epollEvents, MAX_EPOLLEVENT_NUM, -1);
        if(eventNum == -1)
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

bool WebServer::doClientRequest()
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

    return true;
}

void WebServer::closeConnect(int sockfd)
{
#ifdef DEBUG
    printf("%s\n", "close");
#endif
    epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, 0);
    close(sockfd);
}

void WebServer::doClientRead(int sockfd)
{
    // 处理客户端发来的数据
    // 将客户端对应的http对象的数据处理状态设为读取状态
    http_user[sockfd].m_RWStat = http_connect::HTTP_READ_STATUS;
    // 将读取任务放入线程池中执行
    thd_Pool->append_task(read_work, &http_user[sockfd]);
}

void WebServer::doClientWrite(int sockfd)
{
    // 将准备好的数据发送给客户端
    // 将客户端对应的http对象的数据处理状态设为写入状态
    http_user[sockfd].m_RWStat = http_connect::HTTP_WRITE_STATUS;
    // 将写入任务放入线程池中执行
    thd_Pool->append_task(write_work, &http_user[sockfd]);
}

void WebServer::read_work(http_connect* http)
{
    http->read(); 
}


void WebServer::write_work(http_connect* http)
{
    http->write();
}
