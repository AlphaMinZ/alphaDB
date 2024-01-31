#ifndef __ALPHA_LOG_H__
#define __ALPHA_LOG_H__

#include <string>
#include <stdint.h>
#include <iostream>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include <map>
#include "thread.h"
#include "singleton.h"

/**
 * @brief 使用流式方式将日志级别level的日志写入到logger
 */
// #define ALPHA_LOG_LEVEL(logger, level) \
//     if(logger->getLevel() <= level) \
//         alphaMin::LogEventWrap(alphaMin::LogEvent::ptr(new alphaMin::LogEvent(logger, level, \
//                         __FILE__, __LINE__, 0, alphaMin::GetThreadId(),\
//                 alphaMin::GetFiberId(), time(0), alphaMin::Thread::GetName()))).getSS()

#define ALPHA_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level)  \
        alphaMin::LogEventWrap(alphaMin::LogEvent::ptr(new alphaMin::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, 0,\
                0, time(0), alphaMin::Thread::GetName()))).getSS() 

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define ALPHA_LOG_DEBUG(logger) ALPHA_LOG_LEVEL(logger, alphaMin::LogLevel::DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志写入到logger
 */
#define ALPHA_LOG_INFO(logger) ALPHA_LOG_LEVEL(logger, alphaMin::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志写入到logger
 */
#define ALPHA_LOG_WARN(logger) ALPHA_LOG_LEVEL(logger, alphaMin::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志写入到logger
 */
#define ALPHA_LOG_ERROR(logger) ALPHA_LOG_LEVEL(logger, alphaMin::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志写入到logger
 */
#define ALPHA_LOG_FATAL(logger) ALPHA_LOG_LEVEL(logger, alphaMin::LogLevel::FATAL)

/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 */
// #define ALPHA_LOG_FMT_LEVEL(logger, level, fmt, ...) \
//     if(logger->getLevel() <= level) \
//         alphaMin::LogEventWrap(alphaMin::LogEvent::ptr(new alphaMin::LogEvent(logger, level, \
//                         __FILE__, __LINE__, 0, alphaMin::GetThreadId(),\
//                 alphaMin::GetFiberId(), time(0), alphaMin::Thread::GetName()))).getEvent()->format(fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别debug的日志写入到logger
 */
#define ALPHA_LOG_FMT_DEBUG(logger, fmt, ...) ALPHA_LOG_FMT_LEVEL(logger, alphaMin::LogLevel::DEBUG, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别info的日志写入到logger
 */
#define ALPHA_LOG_FMT_INFO(logger, fmt, ...)  ALPHA_LOG_FMT_LEVEL(logger, alphaMin::LogLevel::INFO, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别warn的日志写入到logger
 */
#define ALPHA_LOG_FMT_WARN(logger, fmt, ...)  ALPHA_LOG_FMT_LEVEL(logger, alphaMin::LogLevel::WARN, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别error的日志写入到logger
 */
#define ALPHA_LOG_FMT_ERROR(logger, fmt, ...) ALPHA_LOG_FMT_LEVEL(logger, alphaMin::LogLevel::ERROR, fmt, __VA_ARGS__)

/**
 * @brief 使用格式化方式将日志级别fatal的日志写入到logger
 */
#define ALPHA_LOG_FMT_FATAL(logger, fmt, ...) ALPHA_LOG_FMT_LEVEL(logger, alphaMin::LogLevel::FATAL, fmt, __VA_ARGS__)

/**
 * @brief 获取主日志器
 */
#define ALPHA_LOG_ROOT() alphaMin::LoggerMgr::GetInstance()->getRoot()

/**
 * @brief 获取name的日志器
 */
#define ALPHA_LOG_NAME(name) alphaMin::LoggerMgr::GetInstance()->getLogger(name)

namespace alphaMin {

class Logger;
class LoggerManager;

// 日志级别
class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    // 将日志级别转换成文本输出
    static const char* ToString(LogLevel::Level level);

    // 将文本装换成日志级别
    static LogLevel::Level FromString(const std::string& str);
};

// 日志事件
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id, uint64_t time
            ,const std::string& thread_name);

    // 返回文件名
    const char* getFile() const { return m_file;}

    int32_t getLine() const { return m_line;} 

    uint32_t getElapse() const { return m_elapse;}

    uint32_t getThreadId() const { return m_threadId;}

    uint32_t getFiberId() const { return m_fiberId;}

    uint64_t getTime() const { return m_time;}

    const std::string& getThreadName() const { return m_threadName;}

    std::string getContent() const { return m_ss.str();}

    std::shared_ptr<Logger> getLogger() const { return m_logger;}

    LogLevel::Level getLevel() const { return m_level;}

    std::stringstream& getSS() { return m_ss;}

    // 格式化写入日志内容
    void format(const char* fmt, ...);

    void format(const char* fmt, va_list al);

private:
    // 文件名
    const char* m_file = nullptr;
    // 行号
    int32_t m_line = 0;
    // 程序启动开始到现在的毫秒数
    uint32_t m_elapse = 0;
    // 线程ID
    uint32_t m_threadId = 0;
    // 协程ID
    uint32_t m_fiberId = 0;
    // 时间戳
    uint64_t m_time = 0;
    // 线程名称
    std::string m_threadName;
    // 日志内容流
    std::stringstream m_ss;
    // 日志器
    std::shared_ptr<Logger> m_logger;
    // 日志级别
    LogLevel::Level m_level;
};

// 日志包装器
class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();

    // 获取日志事件
    LogEvent::ptr getEvent() const { return m_event;}

    // 获取日志内容流
    std::stringstream& getSS();
private:
    // 日志事件
    LogEvent::ptr m_event;
};

// 日志格式化
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter(const std::string& pattern); 

    // 返回格式化日志文本
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

public:
    // 日志内容格式化
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        virtual ~FormatItem() {}

        // 格式化日志到流
        virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    // 初始化 解析日志模板
    void init();

    bool isError() const { return m_error;}

    // 返回日志模板
    const std::string getPattern() const { return m_pattern;}
private:
    // 日志格式模板
    std::string m_pattern;
    // 日志格式解释后格式
    std::vector<FormatItem::ptr> m_items;
    // 是否有错
    bool m_error = false;
};

// 日志输出目标
class LogAppender {
friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Spinlock MutexType;

    ~LogAppender() {}

    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

    // 将日志输出目标的配置转换成 YAML String
    // virtual std::string toYamlString() = 0;

    // 更改日志格式器
    void setFormatter(LogFormatter::ptr val);

    // 获取日志格式器
    LogFormatter::ptr getFormatter();

    // 获取日志级别
    LogLevel::Level getLevel() const { return m_level;}

    // 设置日志级别
    void setLevel(LogLevel::Level val) { m_level = val;}
protected:
    // 日志级别
    LogLevel::Level m_level = LogLevel::DEBUG;
    // 是否有自己的日志格式器
    bool m_hasFormatter = false;
    // Mutex
    MutexType m_mutex;
    // 日志格式器
    LogFormatter::ptr m_formatter;
};

// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
friend class LoggerManager;
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    Logger(const std::string& name = "root");

    // 写日志
    void log(LogLevel::Level level, LogEvent::ptr event);

    // 写debug日志
    void debug(LogEvent::ptr event);

    // 写info日志
    void info(LogEvent::ptr event);

    // 写warn日志
    void warn(LogEvent::ptr event);

    // 写error日志
    void error(LogEvent::ptr event);
    
    // 写fatal日志
    void fatal(LogEvent::ptr event);
    
    // 添加日志目标 
    void addAppender(LogAppender::ptr appender);

    // 删除日志目标
    void delAppender(LogAppender::ptr appender);

    // 清除日志目标
    void clearAppenders();

    // 返回日志级别
    LogLevel::Level getLevel() const { return m_level;}

    // 设置日志级别
    void setLevel(LogLevel::Level val) { m_level = val;}

    // 返回日志名称
    const std::string& getName() const { return m_name;}

    // 设置日志格式器
    void setFormatter(LogFormatter::ptr val);

    // 设置日志格式模板
    void setFormatter(const std::string& val);

    // 获取日志格式器
    LogFormatter::ptr getFormatter();

    // 将日志器的配置转成YAML String
    std::string toYamlString();
private:
    // 日志名称
    std::string m_name;
    // 日志级别
    LogLevel::Level m_level;
    // Mutex
    MutexType m_mutex;
    // 日志目标集合
    std::list<LogAppender::ptr> m_appenders;
    // 日志格式器
    LogFormatter::ptr m_formatter;
    // 主日志器
    Logger::ptr m_root;
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    // std::string toYamlString() override;
};

// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
    // std::string toYamlString() override;

    // 重新打开日志文件
    bool reopen();
private:
    // 文件路径
    std::string m_filename;
    // 文件流
    std::ofstream m_filestream;
    // 上次重新打开时间
    uint64_t m_lastTime = 0;
};

// 日志管理器类
class LoggerManager {
public:
    typedef Spinlock MutexType;

    LoggerManager();

    // 获取日志器
    Logger::ptr getLogger(const std::string& name);

    // 初始化
    void init();

    // 返回主日志器
    Logger::ptr getRoot() const { return m_root;}

    // 将所有的日志器配置转成YAML String
    // std::string toYamlString();
private:
    // Mutex
    MutexType m_mutex;
    // 日志器容器
    std::map<std::string, Logger::ptr> m_loggers;
    // 主日志器
    Logger::ptr m_root;
};

// 日志管理器类单例模式
typedef alphaMin::Singleton<LoggerManager> LoggerMgr;

}

#endif