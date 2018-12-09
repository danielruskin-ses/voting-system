#pragma once 

#include <sqlite3.h>
#include <mutex>

#include "Logger.shared.h"
#include "vote_server.grpc.pb.h"

class Database {
public:
        explicit Database(const std::string& databasePath, const Logger& logger);

        ElectionMetadata fetchElectionMetadata();
        std::string fetchVoteServerPublicKey();
        std::string fetchVoteServerPrivateKey();
        void saveConfig(int id, const ElectionMetadata& metadata, const std::string& pubKey, const std::string& privKey);

        std::string fetchVoterDevicePublicKey(int voterDeviceId);
        void saveVoterDevicePublicKey(int voterDeviceId, const std::string& pubKey);
        
        RecordedBallot fetchRecordedBallot(int voterDeviceId);
        void saveRecordedBallot(int voterDeviceId, const RecordedBallot& recordedBallot);

        ~Database() {
                sqlite3_close(_database);
        }

private:
        const Logger& _logger;
        sqlite3* _database;
        std::mutex _mutex;

        void doQueryMultiline(const std::string& query);

        sqlite3_stmt* startQuery(const std::string& query);
        void bindParam(sqlite3_stmt* stmt, int idx, int value);
        void bindParam(sqlite3_stmt* stmt, int idx, const std::string& value);
        bool executeQuery(sqlite3_stmt* stmt);
        int getInt(sqlite3_stmt* stmt, int colIdx);
        std::string getString(sqlite3_stmt* stmt, int colIdx);
        void finalizeQuery(sqlite3_stmt* stmt);

        void checkSqliteResponse(int rc, int desiredRc, sqlite3_stmt* stmt);
};
