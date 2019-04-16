#pragma once

/* Server.h - CPP w/ STL */
#include <vector>
#include <thread>
#include <mutex>
#include <memory>

#include "ThreadPool.h"
#include "Clock.h"
#include "shared_cpp/logger/Logger.h"
#include "shared_cpp/database/Database.h"

/*
To run a server:
1. Create the Server instance.
2. Run start to start the server threads.
3. When you are ready to stop the server, run stop() and/or destroy the server.

Assumptions:
1. Server is managed by a single thread (i.e. start/stop cannot be called concurrently).
*/
class Server {
public:
        Server(std::shared_ptr<const Config> config, std::shared_ptr<Logger> logger, int port) : _database(std::make_shared<Database>(config->dbUser(), config->dbPass(), config->dbHost(), config->dbPort(), config->dbName(), config->dbMigrations())), _config(config), _logger(logger), _port(port), _threadPool(config->numThreads(), _database, _logger, _config), _clock(_database, _logger, _config) { }
        ~Server() { stop(); }

        void start();
        void stop();
        bool isFailed() const { return _failed; }
private:
        std::shared_ptr<Logger> _logger;
        std::shared_ptr<const Config> _config;
        std::shared_ptr<Database> _database;
        int _port;

        bool _running = false;
        bool _failed = false;

        std::thread _connectionsLoopThread;
        std::thread _clockThread;

        std::mutex _connectionsMutex;
        ThreadPool _threadPool;
        Clock _clock;

        void connectionsLoop();
        void clockLoop();
};
