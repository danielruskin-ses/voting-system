#include "Server.h"
#include "Sockets.h"

#include <algorithm>
#include <chrono>

#define MAX_CONNECTIONS 10
#define MAX_WAITING_CONNECTIONS 5
#define CLEANUP_LOOP_DELAY std::chrono::seconds(2)

void Server::start() {
        _failed = false;
        _running = true;
        _connectionsLoopThread = std::thread(&Server::connectionsLoop, this);
        _cleanupLoopThread = std::thread(&Server::cleanupLoop, this);
}

void Server::stop() {
        if(_running || _failed) {
                _running = false;
                _connectionsLoopThread.join();
                _cleanupLoopThread.join();
        }
}

void Server::connectionsLoop() {
        _logger->info("Starting connections loop...");

        // Create new socket
        int mainSock = socket(AF_INET, SOCK_STREAM, 0);
        
        // Create server addr
        sockaddr_in serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(_port);
        serv_addr.sin_addr.s_addr = INADDR_ANY; 

        if(bind(mainSock, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
                _logger->error("Failed to start server!");
                _failed = true;
                return;
        }

        listen(mainSock, MAX_WAITING_CONNECTIONS);
        _logger->info("Server started!");
        
        while(_running && !_failed) {
                switch(checkSocketForData(mainSock)) {
                        case(0):
                        {
                                _logger->info("No data received!");
                                break;
                        }
                        case(-1):
                        {
                                _logger->error("Socket error!");
                                _failed = true;

                                // Ends loop because _failed = true
                                break;
                        }
                        default:
                        {
                                // Accept new client
                                int newSock;
                                unsigned int clientLen;
                                sockaddr_in clientAddr;
                                clientLen = sizeof(sockaddr_in);
                                newSock = accept(mainSock, (sockaddr*) &clientAddr, &clientLen);

                                if(newSock < 0) {
                                        _logger->error("Socket error!");
                                        _failed = true;

                                        // Ends loop because _failed = true
                                        break;
                                }

                                std::lock_guard<std::mutex> guard(_connectionsMutex);
                                if(_connections.size() < MAX_CONNECTIONS) {
                                        _logger->info("New connection established!");
                                        _connections.push_back(std::make_unique<Connection>(_logger, newSock));
                                        _connections.back()->start();
                                } else {
                                        _logger->info("New connection dropped - max reached!");
                                        close(newSock);
                                }
                        }
                }
        }

        _running = false;
}

void Server::cleanupLoop() {
        while(_running && !_failed) {
                // Get rid of any dead connections
                {
                        std::lock_guard<std::mutex> guard(_connectionsMutex);
                        for(int i = _connections.size() - 1; i >= 0; i--) {
                                if(!_connections[i]->isRunning()) {
                                         _logger->info("Cleaning up dead connection!");
                                        _connections.erase(_connections.begin() + i);
                                }
                        }
                }
        
                std::this_thread::sleep_for(CLEANUP_LOOP_DELAY);
        }
        
        _running = false;
}
