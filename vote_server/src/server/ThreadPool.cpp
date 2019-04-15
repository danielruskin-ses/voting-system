#include "ThreadPool.h"
#include "CommandProcessor.h"
#include "shared_c/Definitions.h"

#define CONNECTIONS_LOOP_TIMEOUT_SEC 1
#define MAX_SIZE 2000

void ThreadPool::start() {
        _running = true;

        for(int i = 0; i < _numThreads; i++) {
                _threads.emplace_back(&ThreadPool::threadLoop, this);
        }
}

ThreadPool::~ThreadPool() {
        stop();
}

void ThreadPool::stop() {
        if(_running) {
                _running = false;

                for(int i = 0; i < _numThreads; i++) {
                        _threads[i].join();
                }
                _threads.resize(0);
        }
}

void ThreadPool::newConnection(int sock) {
        std::lock_guard<std::mutex> lock(_connectionsQueueMutex);
        _connectionsQueue.push(sock);
}

void ThreadPool::threadLoop() {
        // Open a db connection
        std::unique_ptr<pqxx::connection> dbConn = _database->getConnection();
        while(_running) {
                // Pull a connection if one is available
                int sock = -1;
                {
                        std::lock_guard<std::mutex> lock(_connectionsQueueMutex);
                        if(!_connectionsQueue.empty()) {
                                sock = _connectionsQueue.front();
                                _connectionsQueue.pop();
                        }
                }
                if(sock == -1) {
                        // No conn yet, sleep and retry
                        _logger->info("No pending connections!  Sleeping...");
                        std::this_thread::sleep_for(std::chrono::seconds(CONNECTIONS_LOOP_TIMEOUT_SEC));
                        continue;
                }

                // Receive Command length over socket
                _logger->info("Receiving command length...");
                unsigned int msgLen;
                int res = socketRecv(sock, (BYTE_T*) &msgLen, sizeof(unsigned int), CONNECTIONS_LOOP_TIMEOUT_SEC);
                if(res < 0) {
                        _logger->info("Socket error!  Dropping connection.");
                        close(sock);
                        continue;
                }
                msgLen = ntohl(msgLen);
                if(msgLen > MAX_SIZE) {
                        _logger->info("Message too large!  Dropping connection");
                        close(sock);
                        continue;
                }

                // Receive Command data over socket
                _logger->info("Receiving command...");
                std::vector<BYTE_T> msgBuf(msgLen);
                res = socketRecv(sock, &(msgBuf[0]), msgLen, CONNECTIONS_LOOP_TIMEOUT_SEC);
                if(res < 0) {
                        _logger->info("Socket error!  Dropping connection.");
                        close(sock);
                        continue;
                }

                // Process Command and transmit response over socket
                _logger->info("Processing command...");
                std::pair<bool, std::vector<BYTE_T>> response = processCommand(msgBuf, *dbConn, *_logger, *_config);
                if(response.first) {
                        _logger->info("Sending response...");

                        // Response size
                        unsigned int respSize = response.second.size();
                        respSize = htonl(respSize);
                        res = socketSend(sock, (BYTE_T*) &respSize, sizeof(unsigned int), CONNECTIONS_LOOP_TIMEOUT_SEC);
                        if(res < 0) {
                                _logger->info("Socket error!  Dropping connection.");
                                close(sock);
                                continue;
                        }

                        // Response data
                        res = socketSend(sock, &(response.second[0]), response.second.size(), CONNECTIONS_LOOP_TIMEOUT_SEC);
                        if(res < 0) {
                                _logger->info("Socket error!  Dropping connection.");
                                close(sock);
                                continue;
                        }

                        _logger->info("Response sent!");
                } else {
                        _logger->error("Response error!  Dropping connection.");
                        continue;
                }
                
                // Re-queue connection for next command
                {
                        std::lock_guard<std::mutex> lock(_connectionsQueueMutex);
                        _connectionsQueue.push(sock);
                }
        }
}
