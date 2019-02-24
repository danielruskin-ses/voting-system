#include "Connection.h"
#include "Sockets.h"

#define CONN_TIMEOUT_SEC 15

void Connection::start() {
        _running = true;
        _failed = false;
        _startedAt = time(NULL);
        _loopThread = std::thread(&Connection::loop, this);
}

Connection::~Connection() {
        stop();
        close(_sock);
}

void Connection::stop() {
        if(_running) {
                _running = false;
                _loopThread.join();
        }
}

void Connection::loop() {
        while(_running && !_failed) {
                switch(checkSocketForData(_sock)) {
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
                                // TODO
                        }
                }

                if(difftime(_startedAt, time(NULL)) >= CONN_TIMEOUT_SEC) {
                        _logger.error("Connection timed out!");
                        _failed = true;
                }
        }

        _running = false;
}
