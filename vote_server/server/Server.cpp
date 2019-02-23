#include "Server.h"

#include <algorithm>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>

void Server::start() {
        _failed = false;
        _running = true;
        _connectionsLoopThread = std::thread(&Server::connectionsLoop, this);
        _cleanupLoopThread = std::thread(&Server::cleanupLoop, this);
}

void Server::stop() {
        if(_running) {
                _running = false;
                _connectionsLoopThread.join();
                _cleanupLoopThread.join();
        }
}

void Server::connectionsLoop() {
        _logger.info("Starting connections loop...");

        // Create new socket
        int mainSock = socket(AF_INET, SOCK_STREAM, 0);
        
        // Create server addr
        sockaddr_in serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(_port);
        serv_addr.sin_addr.s_addr = INADDR_ANY; 

        if(bind(mainSock, (sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
                _logger.error("Failed to start server!");
                _failed = true;
                return;
        }

        listen(mainSock, MAX_WAITING_CONNECTIONS);
        _logger.info("Server started!");
        
        while(_running && !_failed) {
                // Determine if new data is available on socket (wait max 5sec)
                struct timeval tv;
                tv.tv_sec = SOCKET_LOOP_TIMEOUT_SEC; 
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(mainSock, &rfds);

                int recVal = select(mainSock + 1, &rfds, NULL, NULL, &tv);
                switch(recVal) {
                        case(0):
                        {
                                _logger.info("No data received!");
                                break;
                        }
                        case(-1):
                        {
                                _logger.error("Socket error!");
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

                                std::lock_guard<std::mutex> guard(_connectionsMutex);
                                _connections.emplace_back(_logger, newSock);
                                _connections.back().start();
                        }
                }
        }
}

void Server::cleanupLoop() {
        // Get rid of any dead connections
        std::lock_guard<std::mutex> guard(_connectionsMutex);
        for(int i = _connections.size() - 1; i >= 0; i--) {
                if(!_connections[i].isRunning()) {
                        _connections.erase(_connections.begin() + i);
                }
        }
}
