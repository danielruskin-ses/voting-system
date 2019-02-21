#include "server/Server.h"

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

bool Server::connectionsLoop() {
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
                // TODO
                struct timeval tv;
                tv.tv_sec = 10;
                fd_set rfds;
                FD_ZERO(&rfds);
                FD_SET(mainSock, &rfds);
                int recVal = select(mainSock + 1, &rfds, NULL, NULL, &tv);

                // Accept new client
                int newSock;
                unsigned int clientLen;
                sockaddr_in clientAddr;
                clientLen = sizeof(sockaddr_in);
                newSock = accept(mainSock, (sockaddr*) &clientAddr, &clientLen);
        }
}

void Server::cleanupLoop() {

}
