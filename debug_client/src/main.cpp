#include <iostream>
#include <cstdlib>
#include "shared_cpp/logger/Logger.h"
#include "client/Client.h"

#define STDIN 0

int main(int argc, char** argv) {
        std::shared_ptr<Logger> logger(std::make_shared<Logger>(std::cout, std::cerr));

        const char* db_user = std::getenv("DB_USER");
        const char* db_pass = std::getenv("DB_PASS");
        const char* db_host = std::getenv("DB_HOST");
        const char* db_port = std::getenv("DB_PORT");
        const char* db_name = std::getenv("DB_NAME");
        const char* db_migrations = std::getenv("DB_MIGRATIONS");
        const char* server_pubkey = std::getenv("SERVER_PUBKEY");
        const char* client_privkey = std::getenv("CLIENT_PRIVKEY");
        const char* server_host = std::getenv("SERVER_HOST");
        const char* server_port = std::getenv("SERVER_PORT");
        std::shared_ptr<const Config> config = std::make_shared<const Config>(db_user, db_pass, db_host, db_port, db_name, db_migrations, server_pubkey, client_privkey, server_host, server_port);
        if(!config->valid()) {
                logger->error("Invalid config!");
                return 1;
        }

        Client client(config, logger);
        client.start();

        return 0;
}
