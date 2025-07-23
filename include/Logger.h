#pragma once

#include <string>
#include <vector>
#include <mutex>

namespace MasterMind {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class Logger {
public:
    struct LogEntry {
        LogLevel level;
        std::string category;
        std::string message;
        std::string timestamp;
    };

    static Logger& getInstance();
    
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    void setLogLevel(LogLevel level);
    
    void log(LogLevel level, const std::string& message, const std::string& category = "");
    void debug(const std::string& message, const std::string& category = "");
    void info(const std::string& message, const std::string& category = "");
    void warning(const std::string& message, const std::string& category = "");
    void error(const std::string& message, const std::string& category = "");
    void critical(const std::string& message, const std::string& category = "");
    
    std::vector<LogEntry> getRecentEntries(size_t count = 100) const;
    void clearEntries();

private:
    Logger();
    ~Logger();
    
    LogLevel logLevel_;
    std::vector<LogEntry> logEntries_;
    mutable std::mutex mutex_;
};

// Convenience macros
#define LOG_DEBUG(msg, cat) MasterMind::Logger::getInstance().debug(msg, cat)
#define LOG_INFO(msg, cat) MasterMind::Logger::getInstance().info(msg, cat)
#define LOG_WARNING(msg, cat) MasterMind::Logger::getInstance().warning(msg, cat)
#define LOG_ERROR(msg, cat) MasterMind::Logger::getInstance().error(msg, cat)
#define LOG_CRITICAL(msg, cat) MasterMind::Logger::getInstance().critical(msg, cat)

} // namespace MasterMind 