#include "Database.shared.h"

#include <fstream>
#include <cstring>

Database::Database(const std::string& databasePath, const Logger& logger) : _logger(logger) {
        std::lock_guard<std::mutex> guard(_mutex);

        // We do locking ourselves
        sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);

        // Open database
        int dbOpenResp = sqlite3_open((databasePath + "/database.db").c_str(), &_database);
        checkSqliteResponse(dbOpenResp, 0, NULL);

        // Load database setup query
        std::string queryStr;
        std::ifstream setupFile(databasePath + "/database_setup.sql");
        if(setupFile.is_open()) {
                std::string line;
                while(std::getline(setupFile, line)) {
                        queryStr += line + "\n";
                }
        } else {
                throw std::runtime_error("Error opening database setup file!");
        }

        // Execute database setup query
        doQueryMultiline(queryStr);
}

void Database::executeQuery(const std::string& queryStr) {
        sqlite3_stmt* query = startQuery(queryStr);
        executeQuery(query);
        finalizeQuery(query);
}

void Database::checkSqliteResponse(int rc, int desiredRc, sqlite3_stmt* stmt) {
        if(rc != desiredRc) {
                _logger.error("SQLite3 Error: " + std::string(sqlite3_errmsg(_database)));
                finalizeQuery(stmt);
                throw std::runtime_error("SQLite3 Error");
        }
}

// Assumes query has 1 or more statements, and last character is a newline.
void Database::doQueryMultiline(const std::string& query) {
        const char* queryCStr = query.c_str();
        const char* queryEnd = queryCStr + query.size() + 1;

        const char* remQuery = queryCStr;
        while(strlen(remQuery) != 1) {
                sqlite3_stmt* stmt = NULL;
                int resp = sqlite3_prepare_v2(_database, remQuery, -1, &stmt, &remQuery);
                checkSqliteResponse(resp, SQLITE_OK, stmt);
                executeQuery(stmt);
                finalizeQuery(stmt);
        }
}

sqlite3_stmt* Database::startQuery(const std::string& query) {
        sqlite3_stmt* stmt = NULL;
        int resp = sqlite3_prepare_v2(_database, query.c_str(), -1, &stmt, NULL);
        checkSqliteResponse(resp, SQLITE_OK, stmt);

        return stmt;
}

void Database::bindInt(sqlite3_stmt* stmt, int idx, int value) {
        int resp = sqlite3_bind_int(stmt, idx, value);
        checkSqliteResponse(resp, SQLITE_OK, stmt);
}

void Database::bindBlob(sqlite3_stmt* stmt, int idx, const std::string& value) {
        int resp = sqlite3_bind_blob(stmt, idx, value.c_str(), value.size() + 1, SQLITE_TRANSIENT);
        checkSqliteResponse(resp, SQLITE_OK, stmt);
}

bool Database::executeQuery(sqlite3_stmt* stmt) {
        int resp = sqlite3_step(stmt);
        
        if(resp == SQLITE_DONE) {
                // Out of rows
                return false;
        } else if(resp == SQLITE_ROW) {
                // Rows left
                return true;
        } else {
                // Throw error always
                checkSqliteResponse(resp, SQLITE_DONE, stmt);
                return false;
        }
}

int Database::getInt(sqlite3_stmt* stmt, int colIdx) {
        return sqlite3_column_int(stmt, colIdx);
}

std::string Database::getBlob(sqlite3_stmt* stmt, int colIdx) {
        return std::string(
                reinterpret_cast<const char*>(sqlite3_column_blob(stmt, colIdx)),
                sqlite3_column_bytes(stmt, colIdx)
        );
}

void Database::finalizeQuery(sqlite3_stmt* stmt) {
        int resp = sqlite3_finalize(stmt);
        checkSqliteResponse(resp, SQLITE_OK, NULL);
}
