#include "http_connect.h"

int http_connect::epollfd = -1;

void http_connect::init(int fd, const struct sockaddr_in &addr, sqlConnectPool *sqlcnnPool, std::string root, 
                            std::string sqlUser, std::string sqlPwd, std::string sqlDatabaseName)
{
    m_sockfd = fd;
    m_sockfd_addr = addr;

    addfd(epollfd, m_sockfd, false);

    sql_pool = sqlcnnPool;
    m_rootPath = root;
    m_sqlUser = sqlUser;
    m_sqlPwd = sqlPwd;
    m_sqlDatabaseName = sqlDatabaseName;

    init();
}

void http_connect::addfd(int epfd, int fd, bool one_shot)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLRDHUP;

    if(one_shot)
    {
        ev.events |= EPOLLONESHOT;
    }

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblock(fd);
}

void http_connect::init()
{
    mysqlDB = nullptr;
    m_read_idx = 0;
    m_write_idx = 0;
    m_check_idx = 0;
    m_line_idx = 0;
    m_content_len = 0;
    memset(readBuff, 0, READBUFF_MAX_LEN);
    memset(writeBuff, 0, WRITEBUFF_MAX_LEN);

    m_check_state = CHECK_STATE_REQUESTLINE;
    m_currentMethod = GET;
    m_currentURL = nullptr;
    m_head_connectionVal = false;
    m_head_hostVal = nullptr;
    m_content_text = nullptr;
    m_file_address = nullptr;
    m_requestFile = m_rootPath;
}

int http_connect::setnonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void http_connect::read()
{
    // 每读取一次就解析一次
    if(read_once())
    {
        // 读取成功，开始解析数据
        mysqlDB = sql_pool->getConnection();
        HTTP_CODE readParse_ret = read_parse();
         if (readParse_ret == NO_REQUEST)
        {
            modfd(epollfd, m_sockfd, EPOLLIN, 0);
            return;
        }

        // 读取的数据全部解析完成，开始写入数据并发送给客户端
        bool writeParse_ret = write_parse(readParse_ret);
        if (!writeParse_ret)
        {
            close_conn();
        }
        modfd(epollfd, m_sockfd, EPOLLOUT, 0);
    }
}

void http_connect::modfd(int epollfd, int fd, int ev, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

bool http_connect::read_once()
{
    if(m_read_idx > READBUFF_MAX_LEN)
    {
        return false;
    }
    // 本次读取的字节数
    int current_idx = recv(m_sockfd, readBuff + m_read_idx, READBUFF_MAX_LEN - m_read_idx, 0);
    if(current_idx == -1)
    {
        return false;
    }

    m_read_idx += current_idx;
    return true;
}

http_connect::HTTP_CODE http_connect::read_parse()
{
    LINE_STATUS lineStatus = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *line_begin = nullptr;

    // 进行行解析
    while(m_check_state == CHECK_STATE_CONTENT && lineStatus == LINE_OK || (lineStatus = line_end_parse()) == LINE_OK)
    {
        // 得到每一行的起始位置
        line_begin = &readBuff[m_line_idx];
        // 更新下一行的起始位置
        m_line_idx = m_check_idx;

        if(m_check_state == CHECK_STATE_REQUESTLINE)
        {
            // 当前解析行是请求行
            ret = parse_request_line(line_begin);
            if(ret == BAD_REQUEST)
            {
                // 如果如果请求行解析错误，返回错误状态
                return BAD_REQUEST;
            }
        }
        else if(m_check_state == CHECK_STATE_HEADER)
        {
            // 当前解析行是请求头部
            ret = parse_headers(line_begin);
            if(ret == BAD_REQUEST)
            {
                return BAD_REQUEST;
            }
            else if(ret == GET_REQUEST)
            {
                // 头部解析完成，并说明请求数据长度为0，直接做出请求响应
                return do_request();
            }
        }
        else if(m_check_state == CHECK_STATE_CONTENT)
        {
            // 当前解析行是请求数据
            ret = parse_content(line_begin);
            if(ret == GET_REQUEST)
            {
                return do_request();
            }
            lineStatus = LINE_OPEN;
        }
        else
        {
            // 如果无法识别，说明出现其他错误
            return INTERNAL_ERROR;
        }
    }

    return NO_REQUEST;
}

/*
 在http协议中，请求报文和相应报文每一行的结束都是以 回车符+换行符的形式结束的，如：
 GET /index/welcome.html HTTP/1.1\r\t
 xxxxxx:xxxxxr\r\t
 xxxx:xxxx\r\t
 \r\t
 
 因此我们解析行的时候只需要关注 \r、\t 这两个字符即可
*/
http_connect::LINE_STATUS http_connect::line_end_parse()
{
    char ch;
    while(m_check_idx < m_read_idx)
    {
        ch = readBuff[m_check_idx];
        if(ch == '\r')
        {
            if((m_check_idx + 1) == m_read_idx)
            {
                // \r 是这一行最后的字符，接收的数据不完整，等待下一次接收
                return LINE_OPEN;
            }
            else if(readBuff[m_check_idx + 1] == '\n')
            {
                // 如果 \r 后面跟着 \t 说明这一行结束,我们用 "\0\0" 来分割这一行
                readBuff[m_check_idx++] = '\0';
                readBuff[m_check_idx++] = '\0';
                return LINE_OK;
            }
            // 如果 \r 既不是一行的最后一个字符，后面也没有 \n 说明这一行是错误行
            return LINE_BAD;
        }
        else if (ch == '\n')
        {
            if (m_check_idx > 1 && readBuff[m_check_idx - 1] == '\r')
            {
                // 如果 \n 前面跟着 \r 说明这一行结束,我们用 "\0\0" 来分割这一行
                readBuff[m_check_idx++] = '\0';
                readBuff[m_check_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        ++m_check_idx;
    }

    return LINE_OPEN;
}


http_connect::HTTP_CODE http_connect::parse_request_line(char *lineBegin)
{
    // 找到第一个空格的位置
    char* tmp = strpbrk(lineBegin, " ");
    if(!tmp)
    {
        return BAD_REQUEST;
    }
    // 将空格换为 \0 作为第一个字符串的结束
    *tmp++ = '\0';
    // 请求行第一个字符串是请求方法，判断是什么方法
    char *method = lineBegin;
    if(strcasecmp(method, "GET") == 0)
    {
        m_currentMethod = GET;      // 请求方法是 GET
    }
    else if(strcasecmp(method, "POST") == 0)
    {
        m_currentMethod = POST;     // 请求方法是 POST
    }
    else
    {
        return BAD_REQUEST;         // 都不是，返回错误请求
    }
    // 防止有多个连续的空格
    // strspn(s1,s2)用于查找s1后第一个与不在s2出现的字符下标
    // 如："   1s12s23 "后第一个不在" "中出现的字符是"1"，返回下标为 3
    tmp += strspn(tmp, " ");
    // 此时tmp指向的是请求行的第二个字符串：URL
    m_currentURL = tmp;
    // 找到第二个空格位置
    tmp = strpbrk(tmp, " ");
    if(!tmp)
    {
        return BAD_REQUEST;
    }
    *tmp++ = '\0';
    // 同样防止有多个连续的空格
    tmp += strspn(tmp, " ");
    if(strcasecmp(tmp, "HTTP/1.1") != 0)
    {
        // 协议版本不是 http1.1,返回错误请求
        return BAD_REQUEST;
    }
    
    // 清除URL前面的传送协议标识http:// 或 https:// (如果有的话)
    if (strncasecmp(m_currentURL, "http://", 7) == 0)
    {
        m_currentURL += 7;
    }
    else if (strncasecmp(m_currentURL, "https://", 8) == 0)
    {
        m_currentURL += 8;
    }
    // 去掉第一个/前面的IP和端口，拿到目录
    m_currentURL = strchr(m_currentURL, '/');
    if (!m_currentURL || m_currentURL[0] != '/')
    {
        return BAD_REQUEST;
    }

    if (strlen(m_currentURL) == 1)
    {
        //当url为 "/" 时，显示web首页界面
        strcat(m_currentURL, "judge.html");
    }
    // 请求行解析结束，设置解析请求头部
    m_check_state = CHECK_STATE_HEADER;

    return NO_REQUEST;
}

http_connect::HTTP_CODE http_connect::parse_headers(char *lineBegin)
{
    if(lineBegin[0] == '\0')
    {
        // 当请求头部第一个字符就是'\0'，说明这一行可能是请求头部的与请求数据之间的空行
        if(m_content_len != 0)
        {
            m_check_state = CHECK_STATE_CONTENT;    /* 请求头部中标明了请求数据不为0，说明需要去解析请求数据 */
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if(strncasecmp(lineBegin, "Connection:", 11) == 0)
    {
        lineBegin += 11;
        lineBegin += strspn(lineBegin, " ");
        if(strcasecmp(lineBegin, "keep-alive"))
        {
            m_head_connectionVal = true;
        }
    }
    else if(strncasecmp(lineBegin, "Content-length:", 15) == 0)
    {
        lineBegin += 15;
        lineBegin += strspn(lineBegin, " \t");
        m_content_len = atol(lineBegin);
    }
    else if (strncasecmp(lineBegin, "Host:", 5) == 0)
    {
        lineBegin += 5;
        lineBegin += strspn(lineBegin, " \t");
        m_head_hostVal = lineBegin;
    }
    else
    {
#ifdef DEBUG
        std::cout << "o,no! unknow header val: " << lineBegin << std::endl;
#endif
    }
    
    return NO_REQUEST;
}

http_connect::HTTP_CODE http_connect::parse_content(char *lineBegin)
{
    // 当执行到这里时， m_check_idx 的值就不会改变了
    // 因为每次调用read_parse时，循环一直满足m_check_state == CHECK_STATE_CONTENT && lineStatus == LINE_OK
    if(m_read_idx >= (m_content_len + m_check_idx))
    {
        // 当检查到请求数据的最后一个字符时，设置结束符，并保存请求数据的内容
        lineBegin[m_content_len] = '\0';
        m_content_text = lineBegin;
        return GET_REQUEST;
    }

    return NO_REQUEST;
}

/*
 收到客户端完整的请求，对请求数据做出判断
 .找到客户端请求的html文件所在目录
 .到数据库中验证客户端传来的用户名和密码
*/
http_connect::HTTP_CODE http_connect::do_request()
{
#ifdef DEBUG
    std::cout << "requestFile: " << m_requestFile << std::endl;
#endif

    // 如果是登录或者注册页面提交的数据，需要对用户名和密码进行验证，然后决定最中需要返回给客户端的html页面
    // 提取用户名和密码 user=123&passwd=123
    if(m_currentMethod == POST && (strcasecmp(m_currentURL + 1, "loginError.html") == 0 || 
                                    strcasecmp(m_currentURL + 1, "registerError.html") == 0))
    {
        std::string sentence;
        std::string name, pwd;
        int i = 5;
        while(m_content_text[i] != '&') 
            ++i;
        m_content_text[i] = '\0';
        name = &m_content_text[5];
        pwd = &m_content_text[i + 8];

        // 如果是登录，查询用户名和密码是否正确
        if(strcasecmp(m_currentURL + 1, "loginError.html") == 0)
        {
            sentence = "select username, passwd from  user where username=\"";
            sentence.append(name);
            sentence.append("\" && passwd=\"");
            sentence.append(pwd);
            sentence.append("\";");
            MYSQL_RES *res = mysqlDB->inquire(sentence);
            if(!res)
            {
#ifdef DEBUG
                std::cout << "inquire error" << std::endl;
#endif
                return BAD_REQUEST;
            }
            //返回结果集中的列数
            int num_fields = mysql_num_fields(res);

            //返回所有字段结构的数组
            MYSQL_FIELD *fields = mysql_fetch_fields(res);

            //从结果集中获取一行(因为同一个用户名只能有一个用户使用)，将对应的用户名和密码，进行比较
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row && name == row[0] && pwd == row[1])
            {
                // 有这个用户
                // 指定返回给客户端的html文件
                m_requestFile += "/welcome.html";
            }
            else
            {
                // 没有这个用户
                // 指定返回给客户端的html文件
                m_requestFile += "/loginError.html";
            }
        }
        else if(strcasecmp(m_currentURL + 1, "registerError.html") == 0)
        {
            // 如果是注册，先查找用户名是否被占用
            sentence = "select username from  user where username=\"";
            sentence.append(name);
            sentence.append("\"");
            MYSQL_RES *res = mysqlDB->inquire(sentence);
            if(!res)
            {
#ifdef DEBUG
                std::cout << "inquire error" << std::endl;
#endif
                return BAD_REQUEST;
            }
            //返回结果集中的列数
            int num_fields = mysql_num_fields(res);

            //返回所有字段结构的数组
            MYSQL_FIELD *fields = mysql_fetch_fields(res);

            //从结果集中获取一行，如果有数据，说明该用户名已存在
            MYSQL_ROW row = mysql_fetch_row(res);
            if(name == row[0])
            {
                // 有这个用户
                // 指定返回给客户端的html文件
                m_requestFile += "/registerError.html";
            }
            else
            {
                // 没有这个用户,将用户名和密码放到数据库中
                sentence = "insert into user value(\"";
                sentence.append(name);
                sentence.append("\", \"");
                sentence.append(pwd);
                sentence.append("\");");
                if(!mysqlDB->add(sentence))
                {
#ifdef DEBUG
                std::cout << "sql add error" << std::endl;
#endif
                return BAD_REQUEST;
                }

                // 指定返回给客户端的html文件
                m_requestFile += "/judge.html";
            }
        } 
    }
    
    // 如果URL并不是登录也不是注册页面，直接指定用户请求的页面文件
    m_requestFile += m_currentURL;

    // 判断页面文件的状态
    if(stat(m_requestFile.c_str(), &m_file_stat) == -1)
    {
        // 失败，返回没有该文件
        return NO_RESOURCE;
    }
    if(!(m_file_stat.st_mode & S_IROTH))
    {
        // 文件不可读可读
        return FORBIDDEN_REQUEST;
    }
    if(S_ISDIR(m_file_stat.st_mode))
    {
        // 该文件是一个目录
        return BAD_REQUEST;
    }
    
    // 将html文件映射到内存中
    int fd = open(m_requestFile.c_str(), O_RDONLY);
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    return FILE_REQUEST;
}

bool http_connect::write_parse(HTTP_CODE readRet)
{
    if(readRet == INTERNAL_ERROR)
    {

    }

    return true;
}

void http_connect::close_conn(bool real_close)
{
    if (real_close && (m_sockfd != -1))
    {
        printf("close %d\n", m_sockfd);
        epoll_ctl(epollfd, EPOLL_CTL_DEL, m_sockfd, 0);
        close(m_sockfd);
        m_sockfd = -1;
    }
}