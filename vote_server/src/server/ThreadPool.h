#pragma once

#include <thread>
#include <queue>

#include "shared_c/sockets/Sockets.h"
#include "shared_cpp/logger/Logger.h"
#include "shared_cpp/database/Database.h"
#include "../Config.h"

/*
Assumptions:
1. ThreadPool is managed by a single thread (i.e start/stop/newConnection cannot be called from more than one place at once)
*/
class ThreadPool {
public:
        ThreadPool(int numThreads, std::shared_ptr<Database> database, std::shared_ptr<Logger> logger, std::shared_ptr<const Config> config) : _numThreads(numThreads), _database(std::move(database)), _logger(logger), _config(config) {}
        ~ThreadPool();
        
        ThreadPool(const ThreadPool& other) = delete;
        ThreadPool(ThreadPool&& other) = delete;

        void start();
        void stop();
        bool isRunning() const { return _running; }

        void newConnection(int sock);
        
private:
        std::shared_ptr<Database> _database;
        std::shared_ptr<Logger> _logger;
        std::shared_ptr<const Config> _config;

        const int _numThreads;
        std::vector<std::thread> _threads;

        std::mutex _connectionsQueueMutex;
        std::queue<int> _connectionsQueue;

        bool _running = false;

        void threadLoop();
};
