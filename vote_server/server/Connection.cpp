#include "Connection.h"
#include "CommandProcessor.h"

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
                                // Receive Command length over socket
                                unsigned int msgLen;
                                int res = socketRecv(_sock, (uint8_t*) &msgLen, sizeof(unsigned int));
                                msgLen = ntohl(msgLen);
                                
                                if(res < 0) {
                                        socketError(); 
                                        break;
                                }
                                if(msgLen > MAX_SIZE) {
                                        error("Message too large!");
                                        break;
                                }

                                // Receive Command data over socket
                                std::vector<uint8_t> msgBuf(msgLen);
                                res = socketRecv(_sock, &(msgBuf[0]), msgLen);
                                if(res < 0) {
                                        socketError();
                                        break;
                                }

                                // Process Command and transmit response over socket
                                std::vector<uint8_t> response = processCommand(msgBuf);
                                res = socketSend(_sock, &(msgBuf[0]), msgLen);
                                if(res < 0) {
                                        socketError();
                                        break;
                                }
                        }
                }
        }

        _running = false;
}
