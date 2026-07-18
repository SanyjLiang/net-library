#pragma once

#include <string>
#include "noncopyable.h"

// LOG_INFO("%s %d", arg1, arg2)
#define LOG_INFO(logmsgFormat, ...)                         \
    do                                                      \
    {                                                       \
        Logger &logger = Logger::instance();                \
        logger.setLogLevel(INFO);                           \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    }while(0)                                               \

#define LOG_ERROR(logmsgFormat, ...)                        \
    do                                                      \
    {                                                       \
        Logger &logger = Logger::instance();                \
        logger.setLogLevel(ERROR);                          \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    }while(0)                                               \

#define LOG_FATAL(logmsgFormat, ...)                        \
    do                                                      \
    {                                                       \
        Logger &logger = Logger::instance();                \
        logger.setLogLevel(FATAL);                          \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
        exit(-1);                                           \
    }while(0)                                               \

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...)                        \
    do                                                      \
    {                                                       \
        Logger &logger = Logger::instance();                \
        logger.setLogLevel(DEBUG);                          \
        char buf[1024] = {0};                               \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    }while(0)                                               \
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif
//定义日志的级别
enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // core dump信息(程序崩溃,无法正常运行)
    DEBUG, // 调试信息
};

//声明日志类Logger
class Logger : noncopyable
{
public:
    //加关键字static是为了单例模式的实现
    //单例模式就是只有一个实例的类
    //而静态成员函数就是所有对象共享同一个函数
    static Logger& instance();

    //设置日志级别
    void setLogLevel(int level);

    //记录日志
    void log(std::string msg);

private:
    int logLevel_;
};