#include <sqlite3.h>
#include <iostream>
#include <fstream>

#include "Database.h"
#include "vote_server.grpc.pb.h"

void Database::checkSqliteResponse(int rc, int desiredRc) const {
        if(rc != desiredRc) {
                _logger.error("SQLite3 Error: " + std::string(sqlite3_errmsg(_database)));
                throw std::runtime_error("SQLite3 Error");
        }
}

void Database::checkSqliteResponse(int rc, int desiredRc, char* sqliteErr) const {
        if(rc != desiredRc) {
                _logger.error("SQLite3 Error: " + std::string(sqliteErr));
                sqlite3_free(sqliteErr);
                throw std::runtime_error("SQLite3 Error");
        }
}

Database::Database(const std::string& databasePath, const Logger& logger) : _logger(logger) {
        // Make sqlite3 threadsafe
        sqlite3_config(SQLITE_CONFIG_SERIALIZED);

        // Open database
        int dbOpenResp = sqlite3_open((databasePath + "/vote_server.db").c_str(), &_database);
        checkSqliteResponse(dbOpenResp, 0);

        // Load database setup query
        std::string setupQuery;
        std::ifstream setupFile(databasePath + "/database_setup.sql");
        if(setupFile.is_open()) {
                std::string line;
                while(std::getline(setupFile, line)) {
                        setupQuery += line + "\n";
                }
        } else {
                throw std::runtime_error("Error opening database setup file!");
        }

        // Run database setup query
        char* dbSetupErrMsg = NULL;
        int dbSetupResp = sqlite3_exec(_database, setupQuery.c_str(), NULL, 0, &dbSetupErrMsg);
        checkSqliteResponse(dbSetupResp, SQLITE_OK, dbSetupErrMsg);
}

ElectionMetadata Database::fetchElectionMetadata() {
        // Prepare output, and callback to save query results to output
        ElectionMetadata metadata;
        auto sqlCallback = [](void* _arg, int numCols, char** valPtrs, char** namePtrs) -> int {
                if(numCols != 1) {
                        throw("Unexpected number of result columns!");
                }

                ((ElectionMetadata*) _arg)->ParseFromString(valPtrs[0]);

                return 0;
        };

        // Execute query to get most recent election metadata (largest ID)
        // This will run sqlCallback above with the output.
        std::string query = "SELECT SERIALIZED_DATA FROM CONFIG ORDER BY ID DESC LIMIT 1";
        char* queryErrMsg = NULL;
        int queryResp = sqlite3_exec(_database, query.c_str(), sqlCallback, (void*) &metadata, &queryErrMsg);
        checkSqliteResponse(queryResp, SQLITE_OK, queryErrMsg);

        // Validate data was parsed correctly
        if(metadata.elections().size() == 0) {
                throw("Error retrieving election metadata!");
        }

        return metadata;
}

void Database::saveElectionMetadata(int id, const ElectionMetadata& metadata) {
        // Serialize metadata to string
        std::string serializedMetadata;
        metadata.SerializeToString(&serializedMetadata);

        // Prepare query w/ bound params
        std::string query = "INSERT OR REPLACE INTO CONFIG VALUES (?, ?)";
        sqlite3_stmt* stmt = NULL;
        int prepareResp = sqlite3_prepare_v2(_database, query.c_str(), -1, &stmt, NULL);
        checkSqliteResponse(prepareResp, SQLITE_OK);

        // Bind ID, serialized metadata to params
        prepareResp = sqlite3_bind_int(stmt, 1, id);
        checkSqliteResponse(prepareResp, SQLITE_OK);
        prepareResp = sqlite3_bind_text(stmt, 2, serializedMetadata.c_str(), -1, SQLITE_STATIC);
        checkSqliteResponse(prepareResp, SQLITE_OK);

        // Execute query
        int executeResp = sqlite3_step(stmt);
        checkSqliteResponse(executeResp, SQLITE_OK);

        // Finalize query (clears mem)
        sqlite3_finalize(stmt);
}
