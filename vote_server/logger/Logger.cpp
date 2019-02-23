#include "Logger.h"

void Logger::log(std::ostream& stream, const std::string& hdr, const std::string& msg) {
        /* Get time str */
        time_t current_time_a = time(NULL);
        tm current_time_b;
        localtime_r(&current_time_a, &current_time_b);
        char timeBuf[26];
        strftime(timeBuf, 25, "%c", &current_time_b);

        stream << "[" << timeBuf << "] [" << std::this_thread::get_id() << "] [" << hdr << "] " << msg << std::endl;
}
