#pragma once 

/* Logger.h - CPP w/ STL */
#include <mutex>

class Logger {
public:
        /* Assumes info, err stay alive longer than the Logger instance. */
        Logger(std::ostream& info, std::ostream& err) : _info(info), _err(err) { }
        
        void info(const std::string& msg) {
                log(_info, "INFO", msg);
        }

        void error(const std::string& msg) {
                log(_err, "ERROR", msg);
        }

private:
        std::ostream& _info;
        std::ostream& _err;
        std::mutex _lock;

        void log(std::ostream& stream, const std::string& hdr, const std::string& msg);
};
