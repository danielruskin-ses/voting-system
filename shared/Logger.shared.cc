#include <iostream>
#include <iomanip>
#include <ctime>

#include "Logger.shared.h"

void Logger::info(const std::string& message) const {
        log("INFO", message);
}

void Logger::warning(const std::string& message) const {
        log("WARNING", message);
}

void Logger::error(const std::string& message) const {
        log("ERROR", message);
}

void Logger::log(const std::string& type, const std::string& message) const {
        std::lock_guard<std::mutex> guard(_mutex);

        // Get time object
        std::time_t timeObj = std::time(nullptr);
        std::tm timeObjLocal;
        localtime_r(&timeObj, &timeObjLocal);
        
        // Log
        std::cout << "[" << type << "] [" << std::put_time(&timeObjLocal, "%d-%m-%Y %H-%M-%S") << "] " << message << std::endl;
}
