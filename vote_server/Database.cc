#include <sqlite3.h>
#include <iostream>
#include <fstream>

#include "Database.h"
#include "vote_server.grpc.pb.h"

void Database::checkSqliteResponse(int rc, int desiredRc, sqlite3_stmt* stmt) {
        if(rc != desiredRc) {
                _logger.error("SQLite3 Error: " + std::string(sqlite3_errmsg(_database)));
                finalizeQuery(stmt);
                throw std::runtime_error("SQLite3 Error");
        }
}

Database::Database(const std::string& databasePath, const Logger& logger) : _logger(logger) {
        std::lock_guard<std::mutex> guard(_mutex);

        // We do locking ourselves
        sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);

        // Open database
        int dbOpenResp = sqlite3_open((databasePath + "/vote_server.db").c_str(), &_database);
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
        sqlite3_stmt* query = startQuery(queryStr);
        executeQuery(query);
        finalizeQuery(query);
}

ElectionMetadata Database::fetchElectionMetadata() {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT ELECTION_METADATA_SERIALIZED_DATA FROM CONFIG ORDER BY ID DESC LIMIT 1");
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Parse result
        ElectionMetadata metadata;
        metadata.ParseFromString(getString(query, 0));

        // Finalize query
        finalizeQuery(query);

        return metadata;
}

std::string Database::fetchVoteServerPublicKey() {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT VOTE_SERVER_PUBLIC_KEY FROM CONFIG ORDER BY ID DESC LIMIT 1");
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Fetch result
        std::string result = getString(query, 0);

        // Finalize query
        finalizeQuery(query);

        return result;
}

std::string Database::fetchVoteServerPrivateKey() {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT VOTE_SERVER_PRIVATE_KEY FROM CONFIG ORDER BY ID DESC LIMIT 1");
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Fetch result
        std::string result = getString(query, 0);

        // Finalize query
        finalizeQuery(query);

        return result;
}

void Database::saveConfig(int id, const ElectionMetadata& metadata, const std::string& pubKey, const std::string& privKey) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Serialize metadata to string
        std::string serializedMetadata;
        metadata.SerializeToString(&serializedMetadata);

        // Execute query
        sqlite3_stmt* query = startQuery("INSERT OR REPLACE INTO CONFIG VALUES (?, ?, ?, ?)");
        bindParam(query, 1, id);
        bindParam(query, 2, serializedMetadata);
        bindParam(query, 3, pubKey);
        bindParam(query, 4, privKey);
        executeQuery(query);
        finalizeQuery(query);
}

std::string Database::fetchVoterDevicePublicKey(int voterDeviceId) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT PUBLIC_KEY FROM VOTER_DEVICES WHERE ID = ?");
        bindParam(query, 1, voterDeviceId);
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Parse result
        std::string result = getString(query, 0);

        // Finalize query
        finalizeQuery(query);

        return result;
}

void Database::saveVoterDevicePublicKey(int voterDeviceId, const std::string& pubKey) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("INSERT OR REPLACE INTO VOTER_DEVICES VALUES (?, ?)");
        bindParam(query, 1, voterDeviceId);
        bindParam(query, 2, pubKey);
        executeQuery(query);
        finalizeQuery(query);
}

RecordedBallot Database::fetchRecordedBallot(int voterDeviceId) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT SERIALIZED_DATA FROM RECORDED_BALLOTS WHERE VOTER_DEVICE_ID = ?");
        bindParam(query, 1, voterDeviceId);
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Parse result
        RecordedBallot ballot;
        ballot.ParseFromString(getString(query, 0));

        // Finalize query
        finalizeQuery(query);

        return ballot;
}

void Database::saveRecordedBallot(int voterDeviceId, const RecordedBallot& recordedBallot) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Serialize recorded ballot to string
        std::string serializedBallot;
        recordedBallot.SerializeToString(&serializedBallot);

        // Execute query
        sqlite3_stmt* query = startQuery("INSERT OR REPLACE INTO RECORDED_BALLOTS VALUES (?, ?)");
        bindParam(query, 1, voterDeviceId);
        bindParam(query, 2, serializedBallot);
        executeQuery(query);
        finalizeQuery(query);
}

sqlite3_stmt* Database::startQuery(const std::string& query) {
        sqlite3_stmt* stmt = NULL;
        int resp = sqlite3_prepare_v2(_database, query.c_str(), -1, &stmt, NULL);
        checkSqliteResponse(resp, SQLITE_OK, stmt);

        return stmt;
}

void Database::bindParam(sqlite3_stmt* stmt, int idx, int value) {
        int resp = sqlite3_bind_int(stmt, idx, value);
        checkSqliteResponse(resp, SQLITE_OK, stmt);
}

void Database::bindParam(sqlite3_stmt* stmt, int idx, const std::string& value) {
        int resp = sqlite3_bind_text(stmt, idx, value.c_str(), -1, SQLITE_TRANSIENT);
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

std::string Database::getString(sqlite3_stmt* stmt, int colIdx) {
        return std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, colIdx)));
}

void Database::finalizeQuery(sqlite3_stmt* stmt) {
        int resp = sqlite3_finalize(stmt);
        checkSqliteResponse(resp, SQLITE_OK, NULL);
}
