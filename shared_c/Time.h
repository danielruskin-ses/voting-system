#include <time.h>

int getCurrentTime() {
        time_t current_time = time(NULL);
        tm current_time_struct;
        localtime_r(&current_time, &current_time_struct);

        return current_time_struct.tm_sec;
}
