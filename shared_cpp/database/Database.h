#pragma once

#include <string>
#include "sqlpp11/postgresql/connection.h"

/*
Assumptions:
1. A postgresql database is running at the provided host.

It is a good idea to call migrate() after creating a new Database instance.
*/
class Database {
public:
        Database(const std::string& user, const std::string& password, const std::string& host, const std::string& db_name, const std::string& migrations);

        void migrate();
        sqlpp::postgresql::connection& getDb() { return _db; }

private:
        const std::string _user;
        const std::string _password;
        const std::string _host;
        const std::string _db_name;
        const std::string _migrations;
        
        sqlpp::postgresql::connection _db;
};
