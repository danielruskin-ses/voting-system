#pragma once 

#include <sqlite3.h>
#include <mutex>

#include "Logger.shared.h"
#include "Database.shared.h"
#include "vote_server.grpc.pb.h"

class VoteServerDatabase : public Database {
public:
        explicit VoteServerDatabase(const std::string& databasePath, const Logger& logger);

        ElectionMetadata fetchElectionMetadata();
        std::string fetchVoteServerPublicKey();
        std::string fetchVoteServerPrivateKey();
        void saveConfig(int id, const ElectionMetadata& metadata, const std::string& pubKey, const std::string& privKey);

        std::string fetchVoterDevicePublicKey(int voterDeviceId);
        void saveVoterDevicePublicKey(int voterDeviceId, const std::string& pubKey);
        
        RecordedBallot fetchRecordedBallot(int voterDeviceId);
        std::vector<RecordedBallot> fetchRecordedBallotsSorted();
        void saveRecordedBallot(int voterDeviceId, const RecordedBallot& recordedBallot);

        bool isSignedTreeGenerated();
        SignedTree fetchSignedTree();
        void saveSignedTree(int id, const SignedTree& signedTree);
};
