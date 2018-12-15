#pragma once 

#include <sqlite3.h>
#include <mutex>

#include "Logger.shared.h"
#include "Database.shared.h"
#include "audit_server.grpc.pb.h"

class AuditServerDatabase : public Database {
public:
        explicit AuditServerDatabase(const std::string& databasePath, const Logger& logger);

        ElectionMetadata fetchElectionMetadata();

        std::string fetchVoteServerPublicKey();
        std::string fetchAuditServerPublicKey();
        std::string fetchAuditServerPrivateKey();
        void saveConfig(int id, const ElectionMetadata& metadata, const std::string& voteServerPubKey, const std::string& auditServerPubKey, const std::string& auditServerPrivKey);

        std::string fetchVoterDevicePublicKey(int voterDeviceId);
        void saveVoterDevicePublicKey(int voterDeviceId, const std::string& pubKey);

        std::vector<SignedRecordedBallot> fetchSignedRecordedBallotsSorted();
        void saveSignedRecordedBallot(int voterDeviceId, const SignedRecordedBallot& signedRecordedBallot);

        bool isSignedTreeReceived();
        SignedTree fetchSignedTree();
        void saveSignedTree(int id, const SignedTree& signedTree);
};
