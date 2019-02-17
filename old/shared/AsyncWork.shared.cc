#include "AsyncWork.shared.h"
#include <chrono>

AsyncWork::AsyncWork(const Logger& logger) : _logger(logger), _shouldExit(false) {
        _thread = std::thread(&AsyncWork::loopOuter, this);
}

AsyncWork::~AsyncWork() {
        _shouldExit = true;
        
        if(_thread.joinable()) {
                _thread.join();
        }
}

void AsyncWork::loopOuter() {
        _logger.info("AsyncWork thread started");

        std::chrono::seconds sleepDuration(10);
        while(!_shouldExit) {
                _logger.info("AsyncWork loop iteration");
                loopInner();
                std::this_thread::sleep_for(sleepDuration);
        }

        _logger.info("AsyncWork thread ended");
}
