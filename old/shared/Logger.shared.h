#pragma once 

#include <string>
#include <mutex>

class Logger {
public:
        Logger() {
        }

        void info(const std::string& message) const;
        void warning(const std::string& message) const;
        void error(const std::string& message) const;
private:
        void log(const std::string& type, const std::string& message) const;

        mutable std::mutex _mutex;
};
