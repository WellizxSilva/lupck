#pragma once
#include <format>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <thread>

namespace Lupck {

enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3
};

class Logger {
private:
    static inline bool _enabled = false;
    static inline bool _consoleOutput = true;
    static inline LogLevel _minLevel = LogLevel::INFO;
    static inline std::string _logPath = "lupck.log";
    static inline std::mutex _logMutex;

    static std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
           << "." << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    static std::string LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

public:
    static void Initialize(bool enabled, LogLevel level, const std::string& path, bool consoleOutput = true) {
        _enabled = enabled;
        _minLevel = level;
        _logPath = path;
        _consoleOutput = consoleOutput;

        if (_enabled && !_logPath.empty()) {
            try {
                std::filesystem::path p(_logPath);
                if (p.has_parent_path()) {
                    std::filesystem::create_directories(p.parent_path());
                }
            } catch (...) {
            }
        }
    }

    static void Log(LogLevel level, const std::string& component, const std::string& message) {
        if (!_enabled || level < _minLevel) return;

        std::lock_guard<std::mutex> lock(_logMutex);

        // uses thread Id as Id
        auto tid = std::this_thread::get_id();
        std::stringstream tidStr;
        tidStr << tid;

        std::string formatted = std::format("[{}] [{}] [tid:{}] [{}] {}\n",
                                            GetTimestamp(),
                                            LevelToString(level),
                                            tidStr.str(),
                                            component,
                                            message);

        if (_consoleOutput) {
            std::cout << formatted << std::flush;
        }

        if (!_logPath.empty()) {
            std::ofstream file(_logPath, std::ios::app);
            if (file.is_open()) {
                file << formatted;
                file.close();
            }
        }
    }
};

}

#define LUPCK_DEBUG(comp, fmt, ...) Lupck::Logger::Log(Lupck::LogLevel::DEBUG, comp, std::format(fmt, ##__VA_ARGS__))
#define LUPCK_INFO(comp, fmt, ...)  Lupck::Logger::Log(Lupck::LogLevel::INFO,  comp, std::format(fmt, ##__VA_ARGS__))
#define LUPCK_WARN(comp, fmt, ...)  Lupck::Logger::Log(Lupck::LogLevel::WARN,  comp, std::format(fmt, ##__VA_ARGS__))
#define LUPCK_ERROR(comp, fmt, ...) Lupck::Logger::Log(Lupck::LogLevel::ERROR, comp, std::format(fmt, ##__VA_ARGS__))
