#include "Logger.h"

#include <iostream>
#include <ctime>
#include <thread>

void Logger::log(std::ostream& stream, const std::string& hdr, const std::string& msg) {
        /* Get time str */
        time_t current_time_a = time(NULL);
        tm current_time_b;
        localtime_r(&current_time_a, &current_time_b);
        char timeBuf[26];
        strftime(timeBuf, 25, "%c", &current_time_b);

        // Avoid concurrent writes
        std::lock_guard<std::mutex> lock(_lock);
        stream << "[" << timeBuf << "] [" << std::this_thread::get_id() << "] [" << hdr << "] " << msg << std::endl;
}
