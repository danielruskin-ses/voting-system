/* Logger.h - CPP w/o STL */
#include "stdio.h"
#include "time.h"

namespace {

/* Stores the current time in buf.  Buf should have at least 26 bytes. */
void TimeStr(char* buf) {
        time_t current_time_a = time(NULL);
        tm current_time_b;
        localtime_r(&current_time_a, &current_time_b);

        strftime(buf, 25, "%c", &current_time_b);
}

}

class Logger {
public:
        /* Assumes info, err stay alive longer than the Logger instance. */
        Logger(FILE* info, FILE* err) : _info(info), _err(err) { }
        
        void info(const char* msg) {
                char current_time_str[26];
                TimeStr(current_time_str);

                fprintf(_info, "[%s] [%i] [%s] %s\n", current_time_str, std::this_thread::get_id(), "INFO", msg);
        }

        void err(const char* msg) {
                char current_time_str[26];
                TimeStr(current_time_str);

                fprintf(_err, "[%s] [%i] [%s] %s\n", current_time_str, std::this_thread::get_id(), "ERROR", msg);
        }

private:
        FILE* _info;
        FILE* _err;
};
