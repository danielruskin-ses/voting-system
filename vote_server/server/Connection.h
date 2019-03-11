#pragma once

#include <thread>
#include <ctime>
#include <pqxx/pqxx>

#include "Sockets.h"
#include "logger/Logger.h"

/*
Assumptions:
1. Connection is managed by a single thread at a time (i.e. start/stop/move cannot be called concurrently).
*/
class Connection {
public:
        Connection(std::unique_ptr<pqxx::connection> dbConn, std::shared_ptr<Logger> logger, int sock) : _dbConn(std::move(dbConn)), _logger(logger), _sock(sock), _timeoutStart(0) {}
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
        int _sock;

        time_t _timeoutStart; // epoch ms

        bool _running = false;
        bool _failed = false;
        std::thread _loopThread;

        void loop();

        void error(const std::string& msg);
        void socketError() { error("Socket error!"); }

        // String is really a byte array
        std::string receiveMessage();

        template<typename T, T_FIELDS>
        std::optional<T> receiveObjectOverSocket() {
                std::pair<char*, int> msg = receiveMessage();
                if(msg[0] == NULL) {
                        return std::nullopt;
                }

                pb_istream_t pbBuf = pb_istream_from_buffer(msg[0], msg[1]);
                T dest;
                bool res = pb_decode_delimited(&pbBuf, T_FIELDS, &T);
                free(msg[0]);

                if(res) {
                        return dest;
                } else{
                        return std::nullopt;
                }
        }
};
