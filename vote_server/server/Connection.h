#pragma once

#include <thread>

#include "../logger/Logger.h"

/*
Assumptions:
1. Connection is managed by a single thread (i.e. start/stop cannot be called concurrently).
2. The Logger provided to Connection lasts longer than the Connection.
*/
class Connection {
public:
        Connection(Logger& logger, int sock) : _logger(logger), _sock(sock) {}
        ~Connection() { stop(); }

        void start();
        void stop();
        bool isRunning() const { return _running; }
        
private:
        Logger& _logger;
        int _sock;

        bool _running = false;
        std::thread _loopThread;

        void loop();
};
