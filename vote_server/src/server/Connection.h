#pragma once

#include <thread>
#include <ctime>
#include <pqxx/pqxx>

#include "Sockets.h"
#include "shared_cpp/logger/Logger.h"
#include "../Config.h"

/*
Assumptions:
1. Connection is managed by a single thread at a time (i.e. start/stop/move cannot be called concurrently).
*/
class Connection {
public:
        Connection(std::unique_ptr<pqxx::connection> dbConn, std::shared_ptr<Logger> logger, std::shared_ptr<const Config> config, int sock) : _dbConn(std::move(dbConn)), _logger(logger), _config(config), _sock(sock) {}
        ~Connection();
        
        Connection(const Connection& other) = delete;
        Connection(Connection&& other) = delete;

        void start();
        void stop();
        bool isRunning() const { return _running; }
        bool isFailed() const { return _failed; }
        
private:
        std::unique_ptr<pqxx::connection> _dbConn;
        std::shared_ptr<Logger> _logger;
        std::shared_ptr<const Config> _config;
        int _sock;


        bool _running = false;
        bool _failed = false;
        std::thread _loopThread;

        void loop();

        void error(const std::string& msg);
        void socketError() { error("Socket error!"); }
};
