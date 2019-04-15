#include "Server.h"
#include "shared_c/sockets/Sockets.h"

#include <algorithm>
#include <chrono>

#define MAX_WAITING_CONNECTIONS 5

void Server::start() {
        _failed = false;
        _running = true;
        _connectionsLoopThread = std::thread(&Server::connectionsLoop, this);
        _threadPool.start();
}

void Server::stop() {
        if(_running || _failed) {
                _running = false;
                _connectionsLoopThread.join();
                _threadPool.stop();
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
                switch(checkSocketForData(mainSock, SOCKET_LOOP_TIMEOUT)) {
                        case(0):
                        {
                                _logger->info("No new connections!");
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

                                _logger->info("New connection established!");
                                _threadPool.newConnection(newSock);
                        }
                }
        }

        _running = false;
}
