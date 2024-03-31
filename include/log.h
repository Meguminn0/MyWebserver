#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <mutex>

#define LOGFLAG_INFO    0
#define LOGFLAG_DEBUG   1
#define LOGFLAG_WARN    2
#define LOGFLAG_ERROR   3

class log
{
public:
    static log* getInstance()
    {
        static class log instance;
        return &instance;
    }

    bool init(const char* filePath, int logBuffSize = 1024, int maxLines = 500000);

    void writeLog(int LogFlag, const char* format, ...);

private:
    log();
    ~log();

    // 更新文件路径
    void updateDirPath(struct tm* tp);
    // 更新文件路名
    void updateLogFileName(struct tm* tp);
    // 创建目录树
    void craeteDirectories(const char *m_dirPath);

private:
    char m_dirPath[128];    // 文件路径
    char m_name[16];        // 文件名
    char m_logFileName[128];// 文件完整名(路径 + 时间 +文件名)
    char *m_buff;           // 日志缓冲区
    int m_maxLines;         // 文件最大行数
    int m_buffSize;         // 缓冲区大小
    int m_currentLineCnt;   // 文件当前行数
    int m_toDay;            // 当前日期(天)
    int m_fileIndex;        // 当天日志文件的标签
    std::ofstream ofs;     // 文件操作
    std::mutex m_logMutex;  // 互斥量
};

#define LOG_DEBUG(format, ...) log::getInstance()->writeLog(LOGFLAG_DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) log::getInstance()->writeLog(LOGFLAG_INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) log::getInstance()->writeLog(LOGFLAG_WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) log::getInstance()->writeLog(LOGFLAG_ERROR, format, ##__VA_ARGS__)

#endif