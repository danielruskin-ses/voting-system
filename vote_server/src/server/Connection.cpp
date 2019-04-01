#include "Connection.h"
#include "CommandProcessor.h"
#include "shared_c/Definitions.h"

#define MAX_SIZE 1000

void Connection::start() {
        _running = true;
        _failed = false;
        _loopThread = std::thread(&Connection::loop, this);
}

Connection::~Connection() {
        stop();
        close(_sock);
}

void Connection::error(const std::string& msg) {
        _logger->error(msg);
        _failed = true;
}

void Connection::stop() {
        if(_running || _failed) {
                _running = false;
                _loopThread.join();
        }
}

// TODO: rethink broken threading behavior (switch to thread pool behavior)
void Connection::loop() {
        while(_running && !_failed) {
                switch(checkSocketForData(_sock)) {
                        case(0):
                        {
                                _logger->info("No data received!");
                                break;
                        }
                        case(-1):
                        {
                                socketError(); 
                                break;
                        }
                        default:
                        {
                                _logger->info("Data received!");

                                // Receive Command length over socket
                                unsigned int msgLen;
                                int res = socketRecv(_sock, (BYTE_T*) &msgLen, sizeof(unsigned int));
                                msgLen = ntohl(msgLen);
                                
                                if(res < 0) {
                                        socketError(); 
                                        break;
                                }
                                if(msgLen > MAX_SIZE) {
                                        error("Message too large!");
                                        break;
                                }

                                _logger->info("Receiving msg of size: " + std::to_string(msgLen));

                                // Receive Command data over socket
                                std::vector<BYTE_T> msgBuf(msgLen);
                                res = socketRecv(_sock, &(msgBuf[0]), msgLen);
                                if(res < 0) {
                                        socketError();
                                        break;
                                }

                                // Process Command and transmit response over socket
                                std::pair<bool, std::vector<BYTE_T>> response = processCommand(msgBuf, *_dbConn, *_logger, *_config);
                                if(response.first) {
                                        _logger->info("Sending response...");

                                        // Response size
                                        unsigned int respSize = response.second.size();
                                        respSize = htonl(respSize);
                                        res = socketSend(_sock, (BYTE_T*) &respSize, sizeof(unsigned int));
                                        if(res < 0) {
                                                socketError();
                                                break;
                                        }

                                        // Response data
                                        res = socketSend(_sock, &(response.second[0]), response.second.size());
                                        if(res < 0) {
                                                socketError();
                                                break;
                                        }
                                        _logger->info("Response sent!");
                                }
                        }
                }
        }

        _running = false;
}
