#include "Time.h"

int getCurrentTime() {
        time_t current_time = time(NULL);
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec;
}

