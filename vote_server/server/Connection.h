#pragma once

#include <thread>

#include "../logger/Logger.h"

/*
Assumptions:
1. Connection is managed by a single thread at a time (i.e. start/stop/move cannot be called concurrently).
2. The Logger provided to Connection lasts longer than the Connection.
*/
class Connection {
public:
        Connection(Logger& logger, int sock) : _logger(logger), _sock(sock) {}
        ~Connection() { stop(); }
        
        Connection(const Connection& other) = delete;
        Connection(Connection&& other) = delete;

        void start();
        void stop();
        bool isRunning() const { return _running; }
        bool isFailed() const { return _failed; }
        
private:
        Logger& _logger;
        int _sock;

        bool _running = false;
        bool _failed = false;
        std::thread _loopThread;

        void loop();
};
