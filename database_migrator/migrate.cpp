#include <iostream>
#include <memory>

#include "database/Database.h"
#include "logger/Logger.h"

int main(int argc, char** argv) {
        std::shared_ptr<Logger> logger(std::make_shared<Logger>(std::cout, std::cerr));
        if(argc != 6) {
                logger->error("Usage: ./" + std::string(argv[0]) + " user password host db_name migrations");
                exit(-1);
        }

        Database database(argv[1], argv[2], argv[3], argv[4], argv[5]);
        database.migrate();
}
