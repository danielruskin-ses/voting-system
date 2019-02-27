#include <iostream>
#include "logger/Logger.h"
#include "server/Server.h"

#define STDIN 0

int main(int argc, char** argv) {
        std::shared_ptr<Logger> logger(std::make_shared<Logger>(std::cout, std::cerr));

        Server server(logger, 8080);
        server.start();

        logger->info("PRESS ENTER TO EXIT");

        // Exit once data received on stdin
        while(true) {
                if(std::cin.get() == '\n') {
                        break;
                }
        }

        // Stop server
        logger->info("Stopping server...");
        server.stop();

        return 0;
}
