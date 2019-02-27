#include "Connection.h"
#include "Sockets.h"
#include "CommandProcessor.h"

#define CONN_TIMEOUT_SEC 10
#define MAX_SIZE 1000

void Connection::start() {
        _running = true;
        _failed = false;
        _timeoutStart = time(NULL);
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
                                // First byte should be a length indicator
                                // Retrieve and validate the length indicator
                                unsigned int msgLen;
                                int res = socketRecv(_sock, (char*) &msgLen, sizeof(unsigned int));
                                msgLen = ntohl(msgLen);

                                if(res < 0) {
                                        socketError(); 
                                        break;
                                }
                                if(msgLen > MAX_SIZE) {
                                        error("Message too large!");
                                        break;
                                }

                                // Retrieve the message
                                char msgBuf[msgLen];
                                res = socketRecv(_sock, msgBuf, msgLen);
                                if(res < 0) {
                                        socketError(); 
                                        break;
                                }

                                // Process the command
                                res = processCommand(msgBuf, msgLen);
                                if(res < 0) {
                                        error("Failure processing command!");
                                        break;
                                }

                                // Reset timeout if command is successfully processed
                                _timeoutStart = time(NULL);
                        }
                }

                if(difftime(_timeoutStart, time(NULL)) >= CONN_TIMEOUT_SEC) {
                        error("Timeout!");
                        break;
                }
        }

        _running = false;
}
