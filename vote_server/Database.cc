#include <sqlite3.h>
#include <iostream>
#include <fstream>

#include "Database.h"

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
        doQueryMultiline(queryStr);
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
        metadata.ParseFromString(getBlob(query, 0));

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
        std::string result = getBlob(query, 0);

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
        std::string result = getBlob(query, 0);

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
        bindInt(query, 1, id);
        bindBlob(query, 2, serializedMetadata);
        bindBlob(query, 3, pubKey);
        bindBlob(query, 4, privKey);
        executeQuery(query);
        finalizeQuery(query);
}

std::string Database::fetchVoterDevicePublicKey(int voterDeviceId) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT PUBLIC_KEY FROM VOTER_DEVICES WHERE ID = ?");
        bindInt(query, 1, voterDeviceId);
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Parse result
        std::string result = getBlob(query, 0);

        // Finalize query
        finalizeQuery(query);

        return result;
}

void Database::saveVoterDevicePublicKey(int voterDeviceId, const std::string& pubKey) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("INSERT OR REPLACE INTO VOTER_DEVICES VALUES (?, ?)");
        bindInt(query, 1, voterDeviceId);
        bindBlob(query, 2, pubKey);
        executeQuery(query);
        finalizeQuery(query);
}

RecordedBallot Database::fetchRecordedBallot(int voterDeviceId) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT SERIALIZED_DATA FROM RECORDED_BALLOTS WHERE VOTER_DEVICE_ID = ?");
        bindInt(query, 1, voterDeviceId);
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Parse result
        RecordedBallot ballot;
        ballot.ParseFromString(getBlob(query, 0));

        // Finalize query
        finalizeQuery(query);

        return ballot;
}

std::vector<RecordedBallot> Database::fetchRecordedBallotsSorted() {
        std::lock_guard<std::mutex> guard(_mutex);

        std::vector<RecordedBallot> ballots;

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT SERIALIZED_DATA FROM RECORDED_BALLOTS ORDER BY VOTER_DEVICE_ID ASC");

        while(executeQuery(query)) {
                ballots.emplace_back();
                ballots.back().ParseFromString(getBlob(query, 0));
        }
        finalizeQuery(query);

        return ballots;
}

void Database::saveRecordedBallot(int voterDeviceId, const RecordedBallot& recordedBallot) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Serialize recorded ballot to string
        std::string serializedBallot;
        recordedBallot.SerializeToString(&serializedBallot);

        // Execute query
        sqlite3_stmt* query = startQuery("INSERT OR REPLACE INTO RECORDED_BALLOTS VALUES (?, ?)");
        bindInt(query, 1, voterDeviceId);
        bindBlob(query, 2, serializedBallot);
        executeQuery(query);
        finalizeQuery(query);
}

bool Database::isSignedTreeGenerated() {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT SERIALIZED_DATA FROM SIGNED_TREES ORDER BY ID DESC LIMIT 1");
        bool res = executeQuery(query);
        finalizeQuery(query);

        return res;
}

SignedTree Database::fetchSignedTree() {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT SERIALIZED_DATA FROM SIGNED_TREES ORDER BY ID DESC LIMIT 1");
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Parse result
        SignedTree signedTree;
        signedTree.ParseFromString(getBlob(query, 0));

        // Finalize query
        finalizeQuery(query);

        return signedTree;
}

void Database::saveSignedTree(int id, const SignedTree& signedTree) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Serialize recorded ballot to string
        std::string serialized;
        signedTree.SerializeToString(&serialized);

        // Execute query
        sqlite3_stmt* query = startQuery("INSERT OR REPLACE INTO SIGNED_TREES VALUES (?, ?)");
        bindInt(query, 1, id);
        bindBlob(query, 2, serialized);
        executeQuery(query);
        finalizeQuery(query);
}

void Database::executeQuery(const std::string& queryStr) {
        sqlite3_stmt* query = startQuery(queryStr);
        executeQuery(query);
        finalizeQuery(query);
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
