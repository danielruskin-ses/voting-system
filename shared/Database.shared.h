#pragma once 

#include <sqlite3.h>
#include <mutex>

#include "Logger.shared.h"

class Database {
public:
        explicit Database(const std::string& databasePath, const Logger& logger);
        void executeQuery(const std::string& queryStr);

        ~Database() {
                sqlite3_close(_database);
        }

protected:
        const Logger& _logger;
        sqlite3* _database;
        std::mutex _mutex;

        void doQueryMultiline(const std::string& query);

        sqlite3_stmt* startQuery(const std::string& query);
        void bindInt(sqlite3_stmt* stmt, int idx, int value);
        void bindBlob(sqlite3_stmt* stmt, int idx, const std::string& value);
        bool executeQuery(sqlite3_stmt* stmt);
        int getInt(sqlite3_stmt* stmt, int colIdx);
        std::string getBlob(sqlite3_stmt* stmt, int colIdx);
        void finalizeQuery(sqlite3_stmt* stmt);

        void checkSqliteResponse(int rc, int desiredRc, sqlite3_stmt* stmt);
};
