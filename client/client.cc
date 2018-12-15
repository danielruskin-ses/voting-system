#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "vote_server.grpc.pb.h"
#include "audit_server.grpc.pb.h"

#include "VoteServerDatabase.h"
#include "AuditServerDatabase.h"
#include "Crypto.shared.h"

#include <sstream>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

void newElectionMetadata(int startEpoch, int endEpoch, Logger& logger, VoteServerDatabase& voteServerDatabase, AuditServerDatabase& auditServerDatabase) {
        // Create election metadata
        ElectionMetadata metadata;
        metadata.mutable_electionstart()->set_epoch(startEpoch);
        metadata.mutable_electionend()->set_epoch(endEpoch);

        // Add 2 elections, with 2 candidates each
        auto& elections = *metadata.mutable_elections();

        // Election 1
        elections[0] = Election::default_instance();
        elections[0].set_description("President");
        auto& candidates = *(elections[0].mutable_candidates());
        candidates[0] = Candidate::default_instance();
        candidates[0].set_name("Candidate A");
        candidates[1] = Candidate::default_instance();
        candidates[1].set_name("Candidate B");

        // Election 2
        elections[1] = Election::default_instance();
        elections[1].set_description("Secretary");
        candidates = *(elections[1].mutable_candidates());
        candidates[0] = Candidate::default_instance();
        candidates[0].set_name("Candidate C");
        candidates[1] = Candidate::default_instance();
        candidates[1].set_name("Candidate D");

        // Create vote server, audit server crypto keys
        std::string voteServerPublicKey, voteServerPrivateKey, auditServerPublicKey, auditServerPrivateKey;
        GenerateKeyPair(voteServerPublicKey, voteServerPrivateKey);
        GenerateKeyPair(auditServerPublicKey, auditServerPrivateKey);

        // Persist to databases
        voteServerDatabase.saveConfig(1, metadata, voteServerPublicKey, voteServerPrivateKey);
        auditServerDatabase.saveConfig(1, metadata, voteServerPublicKey, auditServerPublicKey, auditServerPrivateKey);

        logger.info("Metadata created and saved to vote server, audit server DBs!");
}

void updateElectionMetadata(int startEpoch, int endEpoch, Logger& logger, VoteServerDatabase& voteServerDatabase, AuditServerDatabase& auditServerDatabase) {
        // Fetch existing data
        ElectionMetadata metadata = voteServerDatabase.fetchElectionMetadata();
        std::string voteServerPublicKey, voteServerPrivateKey, auditServerPublicKey, auditServerPrivateKey;
        voteServerPublicKey = voteServerDatabase.fetchVoteServerPublicKey();
        voteServerPrivateKey = voteServerDatabase.fetchVoteServerPrivateKey();
        auditServerPublicKey = auditServerDatabase.fetchAuditServerPublicKey();
        auditServerPrivateKey = auditServerDatabase.fetchAuditServerPrivateKey();

        // Update existing data
        metadata.mutable_electionstart()->set_epoch(startEpoch);
        metadata.mutable_electionend()->set_epoch(endEpoch);

        // Persist to databases
        voteServerDatabase.saveConfig(1, metadata, voteServerPublicKey, voteServerPrivateKey);
        auditServerDatabase.saveConfig(1, metadata, voteServerPublicKey, auditServerPublicKey, auditServerPrivateKey);

        logger.info("Metadata updated and saved to vote server, audit server DBs!");
}

void createVoterDevice(Logger& logger, VoteServerDatabase& voteServerDatabase, AuditServerDatabase& auditServerDatabase, std::map<int, std::pair<std::string, std::string>>& voterDevices, int& maxId) {
        // Generate crypto keys
        std::string pubKey;
        std::string privKey;
        GenerateKeyPair(pubKey, privKey);

        // Save to db, map
        voteServerDatabase.saveVoterDevicePublicKey(maxId, pubKey);
        auditServerDatabase.saveVoterDevicePublicKey(maxId, pubKey);
        voterDevices[maxId] = std::pair<std::string, std::string>(pubKey, privKey);

        // Incr id for next time
        maxId++;
}

void castBallot(const std::vector<std::string>& cmdParts, Logger& logger, VoteServer::Stub& stub, ClientContext& context, const std::map<int, std::pair<std::string, std::string>>& voterDevices, const std::string& voteServerPubKey, std::map<int, SignedRecordedBallot>& ballots) {
        // Command should have four strings:
        // 1. Command name (cast_ballot)
        // 2. Voter device ID
        // 3. Candidate choice 1
        // 4. Candidate choice 2
        if(cmdParts.size() < 4) {
                logger.error("Must pass in voter device ID, candidate choice 1, and candidate choice 2!");
        }
        int voterDeviceId = std::stoi(cmdParts[1]);
        int candidateChoiceOne = std::stoi(cmdParts[2]);
        int candidateChoiceTwo = std::stoi(cmdParts[3]);

        // Generate and sign proposed ballot
        ProposedBallot proposedBallot;
        proposedBallot.set_voterdeviceid(voterDeviceId);
        proposedBallot.mutable_castat()->set_epoch(std::time(0));
        auto& candidateChoices = *(proposedBallot.mutable_candidatechoices());
        candidateChoices[0] = candidateChoiceOne;
        candidateChoices[1] = candidateChoiceTwo;
        SignedProposedBallot signedProposedBallot;
        signedProposedBallot.mutable_proposedballot()->CopyFrom(proposedBallot);
        SignMessage(
                signedProposedBallot.proposedballot(),
                signedProposedBallot.mutable_signature(),
                voterDevices.at(voterDeviceId).second
        );

        // Transmit signed proposed ballot
        SignedRecordedBallot signedRecordedBallot;
        Status status = stub.CastProposedBallot(&context, signedProposedBallot, &signedRecordedBallot);
        if(!status.ok()) {
                logger.error("RPC failed!");
                return;
        }

        // Validate that the returned recorded ballot contains the correct proposed ballot
        bool sameProposed = google::protobuf::util::MessageDifferencer::Equals(
                signedRecordedBallot.recordedballot().signedproposedballot(),
                signedProposedBallot
        );
        if(!sameProposed) {
                logger.error("Invalid proposed ballot!");
                return;
        }
        
        // Validate that the recorded ballot signature is valid
        bool validSig = VerifyMessage(
                signedRecordedBallot.recordedballot(),
                signedRecordedBallot.signature(),
                voteServerPubKey
        );
        if(!validSig) {
                logger.error("Invalid vote server digital signature!");
                return;
        }

        ballots[voterDeviceId] = signedRecordedBallot;
        logger.error("Ballot cast successfully!");
}

int main(int argc, char* argv[]) {
        Logger logger;

        // Connect to vote server
        VoteServerDatabase voteServerDatabase("../vote_server/database", logger);
        std::shared_ptr<Channel> voteServerChannel(grpc::CreateChannel("0.0.0.0:8001", grpc::InsecureChannelCredentials()));
        std::unique_ptr<VoteServer::Stub> voteServerStub(VoteServer::NewStub(voteServerChannel));
        ClientContext voteServerContext;

        // Connect to audit server
        AuditServerDatabase auditServerDatabase("../audit_server/database", logger);
        std::shared_ptr<Channel> auditServerChannel(grpc::CreateChannel("0.0.0.0:8002", grpc::InsecureChannelCredentials()));
        std::unique_ptr<AuditServer::Stub> auditServerStub(AuditServer::NewStub(auditServerChannel));
        ClientContext auditServerContext;

        // voter device id => voter device pub key, voter device priv key
        std::map<int, std::pair<std::string, std::string>> voterDevices;

        // voter device id => voter device ballot
        int maxId = 0;
        std::map<int, SignedRecordedBallot> ballots;

        while(true) {
                // Fetch command
                std::cout << "Command: " << std::flush;
                std::string cmd;
                std::getline(std::cin, cmd);

                // Split into parts
                std::stringstream cmdStream(cmd);
                std::vector<std::string> cmdParts;
                std::string item;
                while(std::getline(cmdStream, item, ' ')) {
                        cmdParts.push_back(item);
                }
                if(cmdParts.size() == 0) {
                        logger.error("No command!");
                        continue;
                }

                if(cmdParts[0] == "exit") {
                        break;
                } else if(cmdParts[0] == "create_election_metadata") {
                        newElectionMetadata(std::time(0), std::time(0) + 1000, logger, voteServerDatabase, auditServerDatabase);
                } else if(cmdParts[0] == "open_polls") {
                        updateElectionMetadata(std::time(0), std::time(0) + 1000, logger, voteServerDatabase, auditServerDatabase);
                } else if(cmdParts[0] == "close_polls") {
                        updateElectionMetadata(std::time(0) - 10, std::time(0) - 1, logger, voteServerDatabase, auditServerDatabase);
                } else if(cmdParts[0] == "create_voter_device") {
                        createVoterDevice(logger, voteServerDatabase, auditServerDatabase, voterDevices, maxId);
                } else if(cmdParts[0] == "cast_ballot") {
                        castBallot(cmdParts, logger, *voteServerStub, voteServerContext, voterDevices, auditServerDatabase.fetchVoteServerPublicKey(), ballots);
                } else if(cmdParts[0] == "fetch_full_tree") {
                        // TODO
                } else if(cmdParts[0] == "fetch_partial_tree") {
                        // TODO
                } else {
                        logger.error("Invalid command!");
                }
        }
}

