#include "Connection.h"
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
                                // Retrieve a Command
                                std::optional<Command> cmd = receiveObjectOverSocket();
                                if(!cmd) {
                                        socketError();
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

// WARNING: make sure to free the char*!
std::pair<char*, int> Connection::receiveMessage() {
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
        char* msgbuf = malloc(msgLen * sizeof(char));
        res = socketRecv(_sock, msgBuf, msgLen);
        if(res < 0) {
                free(msgBuf);
                return std::pair<NULL, 0>;
        }

        return std::make_pair(msgBuf, msgLen);
}
