#include "VoteServerDatabase.h"

VoteServerDatabase::VoteServerDatabase(const std::string& databasePath, const Logger& logger) : Database(databasePath, logger) {
}

ElectionMetadata VoteServerDatabase::fetchElectionMetadata() {
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

std::string VoteServerDatabase::fetchVoteServerPublicKey() {
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

std::string VoteServerDatabase::fetchVoteServerPrivateKey() {
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

void VoteServerDatabase::saveConfig(int id, const ElectionMetadata& metadata, const std::string& pubKey, const std::string& privKey) {
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

std::string VoteServerDatabase::fetchVoterDevicePublicKey(int voterDeviceId) {
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

void VoteServerDatabase::saveVoterDevicePublicKey(int voterDeviceId, const std::string& pubKey) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("INSERT OR REPLACE INTO VOTER_DEVICES VALUES (?, ?)");
        bindInt(query, 1, voterDeviceId);
        bindBlob(query, 2, pubKey);
        executeQuery(query);
        finalizeQuery(query);
}

SignedRecordedBallot VoteServerDatabase::fetchSignedRecordedBallot(int voterDeviceId) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT SERIALIZED_DATA FROM SIGNED_RECORDED_BALLOTS WHERE VOTER_DEVICE_ID = ?");
        bindInt(query, 1, voterDeviceId);
        bool res = executeQuery(query);
        if(!res) {
                finalizeQuery(query);
                throw std::runtime_error("No rows returned!");
        }

        // Parse result
        SignedRecordedBallot ballot;
        ballot.ParseFromString(getBlob(query, 0));

        // Finalize query
        finalizeQuery(query);

        return ballot;
}

std::vector<SignedRecordedBallot> VoteServerDatabase::fetchSignedRecordedBallotsSorted() {
        std::lock_guard<std::mutex> guard(_mutex);

        std::vector<SignedRecordedBallot> ballots;

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT SERIALIZED_DATA FROM SIGNED_RECORDED_BALLOTS ORDER BY VOTER_DEVICE_ID ASC");

        while(executeQuery(query)) {
                ballots.emplace_back();
                ballots.back().ParseFromString(getBlob(query, 0));
        }
        finalizeQuery(query);

        return ballots;
}

void VoteServerDatabase::saveSignedRecordedBallot(int voterDeviceId, const SignedRecordedBallot& signedRecordedBallot) {
        std::lock_guard<std::mutex> guard(_mutex);

        // Serialize recorded ballot to string
        std::string serializedBallot;
        signedRecordedBallot.SerializeToString(&serializedBallot);

        // Execute query
        sqlite3_stmt* query = startQuery("INSERT OR REPLACE INTO SIGNED_RECORDED_BALLOTS VALUES (?, ?)");
        bindInt(query, 1, voterDeviceId);
        bindBlob(query, 2, serializedBallot);
        executeQuery(query);
        finalizeQuery(query);
}

bool VoteServerDatabase::isSignedTreeGenerated() {
        std::lock_guard<std::mutex> guard(_mutex);

        // Execute query
        sqlite3_stmt* query = startQuery("SELECT SERIALIZED_DATA FROM SIGNED_TREES ORDER BY ID DESC LIMIT 1");
        bool res = executeQuery(query);
        finalizeQuery(query);

        return res;
}

SignedTree VoteServerDatabase::fetchSignedTree() {
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

void VoteServerDatabase::saveSignedTree(int id, const SignedTree& signedTree) {
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
