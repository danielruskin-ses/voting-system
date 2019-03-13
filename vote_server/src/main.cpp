#include <iostream>
#include "shared_cpp/logger/Logger.h"
#include "server/Server.h"

#define STDIN 0

int main(int argc, char** argv) {
        std::shared_ptr<Logger> logger(std::make_shared<Logger>(std::cout, std::cerr));

        if(argc != 6) {
                logger->error("Usage: ./" + std::string(argv[0]) + " [user] [password] [host] [db_name] [migrations_dir]");
                return 1;
        }

        Server server(Database(argv[1], argv[2], argv[3], argv[4], argv[5]), logger, 8080);
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
