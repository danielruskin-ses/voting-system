#pragma once

#include <thread>
#include <queue>

#include "shared_cpp/logger/Logger.h"
#include "shared_cpp/database/Database.h"
#include "../Config.h"

/*
Assumptions:
1. Clock is managed by a single thread (i.e tick cannot be called from more than one place at once)
*/
class Clock {
public:
        Clock(std::shared_ptr<Database> database, std::shared_ptr<Logger> logger, std::shared_ptr<const Config> config) : _database(std::move(database)), _logger(logger), _config(config), _dbConn(_database->getConnection()) {}
        
        Clock(const Clock& other) = delete;
        Clock(Clock&& other) = delete;

        void tick();
        
private:
        std::shared_ptr<Database> _database;
        std::unique_ptr<pqxx::connection> _dbConn;
        std::shared_ptr<Logger> _logger;
        std::shared_ptr<const Config> _config;

        void generateTallies();
};
