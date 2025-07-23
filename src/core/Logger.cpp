#include "Logger.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace MasterMind {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : logLevel_(LogLevel::INFO) {
    // Initialize logging system
}

Logger::~Logger() {
    // Cleanup
}

void Logger::setLogLevel(LogLevel level) {
    logLevel_ = level;
}

void Logger::log(LogLevel level, const std::string& message, const std::string& category) {
    if (level < logLevel_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::time(nullptr);
    auto* tm = std::localtime(&now);
    
    std::ostringstream timestamp;
    timestamp << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    
    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG: levelStr = "DEBUG"; break;
        case LogLevel::INFO: levelStr = "INFO"; break;
        case LogLevel::WARNING: levelStr = "WARNING"; break;
        case LogLevel::ERROR: levelStr = "ERROR"; break;
        case LogLevel::CRITICAL: levelStr = "CRITICAL"; break;
    }
    
    std::ostringstream logMessage;
    logMessage << "[" << timestamp.str() << "] [" << levelStr << "] ";
    if (!category.empty()) {
        logMessage << "[" << category << "] ";
    }
    logMessage << message;
    
    std::cout << logMessage.str() << std::endl;
    
    // Store for GUI display if needed
    logEntries_.push_back({level, category, message, timestamp.str()});
    
    // Keep only last 1000 entries to prevent memory issues
    if (logEntries_.size() > 1000) {
        logEntries_.erase(logEntries_.begin());
    }
}

void Logger::debug(const std::string& message, const std::string& category) {
    log(LogLevel::DEBUG, message, category);
}

void Logger::info(const std::string& message, const std::string& category) {
    log(LogLevel::INFO, message, category);
}

void Logger::warning(const std::string& message, const std::string& category) {
    log(LogLevel::WARNING, message, category);
}

void Logger::error(const std::string& message, const std::string& category) {
    log(LogLevel::ERROR, message, category);
}

void Logger::critical(const std::string& message, const std::string& category) {
    log(LogLevel::CRITICAL, message, category);
}

std::vector<Logger::LogEntry> Logger::getRecentEntries(size_t count) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (logEntries_.size() <= count) {
        return logEntries_;
    }
    
    return std::vector<LogEntry>(logEntries_.end() - count, logEntries_.end());
}

void Logger::clearEntries() {
    std::lock_guard<std::mutex> lock(mutex_);
    logEntries_.clear();
}

} // namespace MasterMind 