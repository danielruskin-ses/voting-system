#pragma once

#include "../Config.h"
#include "shared_cpp/logger/Logger.h"
#include "shared_cpp/database/Database.h"

#include "gen_c/pb/shared.pb.h"

class Client {
public:
        Client(std::shared_ptr<const Config> config, std::shared_ptr<Logger> logger) : _database(config->dbUser(), config->dbPass(), config->dbHost(), config->dbPort(), config->dbName(), config->dbMigrations()), _config(config), _logger(logger) { }

        void start();
private:
        std::shared_ptr<Logger> _logger;
        std::shared_ptr<const Config> _config;
        Database _database;

        bool sendCommand(int sock, CommandType commandType, const std::vector<BYTE_T>& data) const;
        std::pair<bool, Response> getResponse(int sock) const;
        void getElections(int sock) const;
        std::tuple<bool, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>> createKeypair();
};
