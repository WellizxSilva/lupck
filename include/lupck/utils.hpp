#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <ctime>

namespace Lupck::Utils {
    class Logger {
    public:
        explicit Logger(const std::string& logFile)
            : logStream(logFile, std::ios::out | std::ios::app) {
            if (!logStream) {
                std::cerr << "Failed to open log file: " << logFile << std::endl;
            }
        }

        void info(const std::string& msg) {
            write("INFO", msg);
        }

        void warn(const std::string& msg) {
            write("WARN", msg);
        }

        void error(const std::string& msg) {
            write("ERROR", msg);
        }

        void success(const std::string& msg) {
            write("SUCCESS", msg);
        }

    private:
        std::ofstream logStream;

        std::string timestamp() {
            std::time_t t = std::time(nullptr);
            char buf[32];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
            return buf;
        }

        void write(const std::string& level, const std::string& msg) {
            std::string line = "[" + timestamp() + "] [" + level + "] " + msg;
            std::cout << line << std::endl;
            if (logStream) {
                logStream << line << std::endl;
            }
        }
    };

    std::string GetExecutablePath();
}
