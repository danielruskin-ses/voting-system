#pragma once 

#include <sqlite3.h>
#include "Logger.shared.h"
#include "vote_server.grpc.pb.h"

class Database {
public:
        explicit Database(const std::string& databasePath, const Logger& logger);

        ElectionMetadata fetchElectionMetadata();
        void saveElectionMetadata(int id, const ElectionMetadata& metadata);

        std::string fetchVoterDevicePublicKey(int voterDeviceId);
        void saveVoterDevicePublicKey(int voterDeviceId, const std::string& pubKey);
        
        RecordedBallot fetchRecordedBallot(int voterDeviceId);
        void saveRecordedBallot(const RecordedBallot& recordedBallot);

        ~Database() {
                sqlite3_close(_database);
        }

private:
        const Logger& _logger;
        sqlite3* _database;

        void checkSqliteResponse(int rc, int desiredRc) const;
        void checkSqliteResponse(int rc, int desiredRc, char* sqliteErr) const;
};
