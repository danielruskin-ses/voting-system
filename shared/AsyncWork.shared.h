#pragma once 

#include <mutex>
#include <thread>
#include <atomic>

#include "Logger.shared.h"

class AsyncWork {
public:
        explicit AsyncWork(const Logger& logger);
        virtual ~AsyncWork();

        virtual void loopInner() = 0;

protected:
        const Logger& _logger;

private:
        std::thread _thread;
        std::atomic<bool> _shouldExit;

        void loopOuter();
};
