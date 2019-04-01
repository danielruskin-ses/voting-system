#pragma once

#include <string>
#include <pqxx/pqxx>

#define MIGRATIONS_TABLE "migrations"

/*
Assumptions:
1. A postgresql database is running at the provided host.
2. The db associated with db_name has the following table: "CREATE TABLE migrations(NAME VARCHAR PRIMARY KEY)"

It is a good idea to call migrate() after creating a new Database instance.
*/
class Database {
public:
        Database(const std::string& user, const std::string& password, const std::string& host, const std::string& port, const std::string& db_name, const std::string& migrations);

        void migrate();
        std::unique_ptr<pqxx::connection> getConnection() const;

private:
        const std::string _connStr;
        const std::string _migrations;
};
