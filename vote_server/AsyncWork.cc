#include "AsyncWork.h"

AsyncWork::AsyncWork(const Logger& logger, Database& database) : _logger(logger), _database(database), _shouldExit(false) {
        _thread = std::thread(&AsyncWork::loop, this);
}

AsyncWork::~AsyncWork() {
        _shouldExit = true;
        
        if(_thread.joinable()) {
                _thread.join();
        }
}

void AsyncWork::loop() {
        std::chrono::seconds sleepDuration(10);

        while(!_shouldExit) {
                // TODO: gen tree

                // Sleep for 10s
                std::this_thread::sleep_for(sleepDuration);
        }
}
