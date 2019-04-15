#include <iostream>
#include <cstdlib>
#include "shared_cpp/logger/Logger.h"
#include "server/Server.h"

#define STDIN 0

int main(int argc, char** argv) {
        std::shared_ptr<Logger> logger(std::make_shared<Logger>(std::cout, std::cerr));

        const char* num_threads = std::getenv("NUM_THREADS");
        const char* db_user = std::getenv("DB_USER");
        const char* db_pass = std::getenv("DB_PASS");
        const char* db_host = std::getenv("DB_HOST");
        const char* db_port = std::getenv("DB_PORT");
        const char* db_name = std::getenv("DB_NAME");
        const char* db_migrations = std::getenv("DB_MIGRATIONS");
        const char* privkey = std::getenv("PRIV_KEY");
        const char* pallier_pubkey = std::getenv("PAILLIER_PUB_KEY");
        const char* pallier_privkey = std::getenv("PAILLIER_PRIV_KEY");
        std::shared_ptr<const Config> config = std::make_shared<const Config>(num_threads, db_user, db_pass, db_host, db_port, db_name, db_migrations, privkey, pallier_pubkey, pallier_privkey);
        if(!config->valid()) {
                logger->error("Invalid config!");
                return 1;
        }

        Server server(config, logger, 8080);
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
