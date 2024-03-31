#include <time.h>
#include <fstream>
#include <cstring>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

#include "log.h"

log::log()
{
    m_currentLineCnt = 0;
    m_fileIndex = 1;
}

log::~log()
{
    if(ofs.is_open())
    {
        ofs.close();
    }

    delete[] m_buff;
}

void log::updateDirPath(struct tm* tp)
{
    int sz = strlen(m_dirPath) - 12;
    // xxxx/config/2021_06_10/ 只需要修改 2021_06_10/
    snprintf(m_dirPath + sz, 128 - strlen(m_dirPath) + 11, "%d_%02d_%02d/", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday);
}

void log::updateLogFileName(struct tm* tp)
{
    snprintf(m_logFileName, 128, "%s%s_%d_%02d_%02d(%d)", m_dirPath, m_name, tp->tm_year + 1900, 
                                tp->tm_mon + 1, tp->tm_mday, ++m_fileIndex);
}


void log::craeteDirectories(const char *m_dirPath)
{
    struct stat st;
    if(stat(m_dirPath, &st) != 0)
    {
        mkdir(m_dirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (errno == ENOENT) 
        {
            // 如果父目录不存在
            char *pos = const_cast<char*>(m_dirPath);
            while ((pos = strchr(pos + 1, '/')) != nullptr) 
            {
                // 分割路径，一次创建一层目录
                *pos = '\0';
                mkdir(m_dirPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                // 恢复分割之前的路径
                *pos = '/'; 
            }
        }
    }
}

bool log::init(const char* filePath, int logBuffSize, int maxLines)
{
    m_buffSize = logBuffSize;
    m_buff = new char[m_buffSize];
    m_maxLines = maxLines;
    int sz;

    time_t timep = time(nullptr);
    struct tm *tp = localtime(&timep);

    const char* name = strrchr(filePath, '/');

    if(name == nullptr)
    {
        // 没有填写日志文件路径
        snprintf(m_name, 16, "Log");
        snprintf(m_dirPath, 128,"./config/%d_%02d_%02d/", tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday);
    }
    else
    {
        snprintf(m_name, 16, "%s", name);
        sz = name - filePath + 1;
        strncpy(m_dirPath, filePath, sz);
        snprintf(m_dirPath + sz, 128, "%d_%02d_%02d/", tp->tm_year + 1900, 
                                tp->tm_mon + 1, tp->tm_mday);
    }
    sz = snprintf(m_logFileName, 128, "%s%s_%d_%02d_%02d(%d)", m_dirPath, m_name, tp->tm_year + 1900, 
                                tp->tm_mon + 1, tp->tm_mday, m_fileIndex);
    m_logFileName[sz] = '\0';
    // 创建目录树
    craeteDirectories(m_dirPath);

    m_toDay = tp->tm_mday;
    while(true)
    {
        ofs.open(m_logFileName, std::ios::out | std::ios::app);
        if(ofs.is_open())
        {
            // 判断文件中是否有数据
            ofs.seekp(0, std::ios::beg);
            int currpos = ofs.tellp();
            ofs.seekp(0, std::ios::end);
            int endpos = ofs.tellp();
            if((endpos - currpos) == 0)
            {
                break;
            }
            // 有数据，则打开新的文件
            updateLogFileName(tp);
            ofs.close();
        }
        else
        {
            return false;
        }
    }

    return true;
}

void log::writeLog(int LogFlag, const char* format, ...)
{
    time_t timep;
    struct tm* tp;
    struct tm localTm;
    time(&timep);
    tp = localtime_r(&timep, &localTm);
    char flag[10];
    switch(LogFlag)
    {
    case LOGFLAG_INFO:
        strcpy(flag, "[INFO]:");
        break;
    case LOGFLAG_DEBUG:
        strcpy(flag, "[DEBUG]:");
        break;
    case LOGFLAG_WARN:
        strcpy(flag, "[WARN]:");
        break;
    case LOGFLAG_ERROR:
        strcpy(flag, "[ERROR]:");
        break;
    }

    // 到了第二天或者文件写满了，更换文件
    m_logMutex.lock();
    // printf("%d, %d\n", m_currentLineCnt, m_maxLines);
    if(m_currentLineCnt++ > m_maxLines || tp->tm_mday != m_toDay)
    {
        if(tp->tm_mday != m_toDay)
        {
            m_toDay = tp->tm_mday;
            updateDirPath(tp);
            m_fileIndex = 1;
        }
        
        m_currentLineCnt = 0;
        updateLogFileName(tp);
        ofs.close();
        ofs.open(m_logFileName, std::ios::out | std::ios::app);
    }
    m_logMutex.unlock();

    // 写入数据
    m_logMutex.lock();
    memset(m_buff, 0, m_buffSize);
    int titleSize = snprintf(m_buff, m_buffSize, "%d-%02d-%02d %02d:%02d:%02d %s ", tp->tm_year + 1900, tp->tm_mon + 1,
                            tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, flag);
    va_list valst;
    va_start(valst, format);
    int contentSize = vsnprintf(m_buff + titleSize, m_buffSize, format, valst);
    m_buff[titleSize + contentSize] = '\n';
    m_buff[titleSize + contentSize + 1] = '\0';

    ofs << m_buff;
    ofs.flush();
    va_end(valst);
    m_logMutex.unlock();
}