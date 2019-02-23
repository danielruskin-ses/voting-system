#include "server/Connection.h"

void Connection::start() {
        _running = true;
        _loopThread = std::thread(&Connection::loop, this);
}

void Connection::stop() {
        if(_running) {
                _running = false;
                _loopThread.join();
        }
}

void Connection::loop() {
        // TODO
}
