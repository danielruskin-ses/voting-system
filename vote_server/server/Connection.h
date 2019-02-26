#pragma once

#include <thread>
#include <ctime>

#include "../logger/Logger.h"

/*
Assumptions:
1. Connection is managed by a single thread at a time (i.e. start/stop/move cannot be called concurrently).
*/
class Connection {
public:
        Connection(std::shared_ptr<Logger> logger, int sock) : _logger(logger), _sock(sock), _startedAt(0) {}
        ~Connection();
        
        Connection(const Connection& other) = delete;
        Connection(Connection&& other) = delete;

        void start();
        void stop();
        bool isRunning() const { return _running; }
        bool isFailed() const { return _failed; }
        
private:
        std::shared_ptr<Logger> _logger;
        int _sock;
        time_t _startedAt; // epoch ms

        bool _running = false;
        bool _failed = false;
        std::thread _loopThread;

        void loop();

        void error(const std::string& msg);
        void socketError() { error("Socket error!"); }
};
