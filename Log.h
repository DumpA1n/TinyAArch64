#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <vector>
#include <string>
#include <mutex>
#include <cstdarg>

class Logger {
public:
    enum LogLevel {
        INFO,
        ERROR
    };

    Logger() : logToFile(false) {}

    // 启用日志文件输出
    void enableFileOutput(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        logToFile = true;
        logFile.open(filename, std::ios::app);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        logs.clear();
    }

    std::vector<std::string> get() {
        std::lock_guard<std::mutex> lock(mutex_);
        return logs;
    }

    void i(const char* format, ...) {
        va_list args;
        va_start(args, format);
        add(INFO, format, args);
        va_end(args);
    }

    void e(const char* format, ...) {
        va_list args;
        va_start(args, format);
        add(ERROR, format, args);
        va_end(args);
    }

private:
    std::mutex mutex_;
    std::vector<std::string> logs;
    bool logToFile;
    std::ofstream logFile;

    void add(LogLevel level, const char* format, va_list args) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 先尝试使用动态字符串缓冲
        std::vector<char> dynamicBuffer(4096); // 初始 4KB
        int len = vsnprintf(dynamicBuffer.data(), dynamicBuffer.size(), format, args);

        if (len < 0) return; // 格式化失败

        if (static_cast<size_t>(len) >= dynamicBuffer.size()) {
            // 缓冲区不够，扩大到需要的大小 +1（用于 '\0'）
            dynamicBuffer.resize(len + 1);
            vsnprintf(dynamicBuffer.data(), dynamicBuffer.size(), format, args);
        }

        std::string logEntry = getTimeStamp() + " " + levelToString(level) + " " + dynamicBuffer.data();
        logs.push_back(logEntry);
        // std::cout << logEntry << std::endl;

        if (logToFile && logFile.is_open()) {
            logFile << logEntry << std::endl;
        }
    }

    std::string getTimeStamp() const {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string levelToString(LogLevel level) const {
        switch (level) {
            case INFO:  return "[I]";
            case ERROR: return "[E]";
            default:    return "[?]";
        }
    }
};

#include <unordered_map>
#include <memory>
#include <string>

class LogManager {
public:
    static LogManager& instance() {
        static LogManager instance;
        return instance;
    }

    std::shared_ptr<Logger> get(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (loggers.find(name) == loggers.end()) {
            loggers[name] = std::make_shared<Logger>();
        }
        return loggers[name];
    }

    void setDefault(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);
        defaultLogger = get(name);
    }

    std::shared_ptr<Logger> getDefault() {
        std::lock_guard<std::mutex> lock(mutex_);
        return defaultLogger;
    }

private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Logger>> loggers;
    std::shared_ptr<Logger> defaultLogger;
};



#define LOG_INSTANCE(name) LogManager::instance().get(name)
#define LOG_DEFAULT        LogManager::instance().getDefault()

#define LOGI(log, fmt, ...) log->i(fmt, ##__VA_ARGS__)
#define LOGE(log, fmt, ...) log->e(fmt, ##__VA_ARGS__)
