#pragma once 

#include <mutex>
#include <thread>
#include <chrono>

#include "Logger.shared.h"
#include "Database.h"
#include "vote_server.grpc.pb.h"

class AsyncWork {
public:
        explicit AsyncWork(const Logger& logger, Database& database);

        ~AsyncWork();
private:
        const Logger& _logger;
        Database& _database;

        std::thread _thread;
        std::atomic<bool> _shouldExit;

        void loop();
};
