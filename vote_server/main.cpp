#include <iostream>
#include "logger/Logger.h"
#include "server/Server.h"

int main(int argc, char** argv) {
        std::shared_ptr<Logger> logger(std::make_shared<Logger>(std::cout, std::cerr));

        Server server(logger, 8081);
        server.start();

        while(true) {
        
        }
}
