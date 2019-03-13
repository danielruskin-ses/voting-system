#pragma once

/* Server.h - CPP w/ STL */
#include <vector>
#include <thread>
#include <mutex>
#include <memory>

#include "Connection.h"
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
        Server(Database&& database, std::shared_ptr<Logger> logger, int port) : _database(database), _logger(logger), _port(port) { }
        ~Server() { stop(); }

        void start();
        void stop();
        bool isFailed() const { return _failed; }
private:
        std::shared_ptr<Logger> _logger;
        Database _database;
        int _port;

        bool _running = false;
        bool _failed = false;

        std::thread _connectionsLoopThread;
        std::thread _cleanupLoopThread;

        std::mutex _connectionsMutex;
        std::vector<std::unique_ptr<Connection>> _connections;

        void connectionsLoop();
        void cleanupLoop();
};
