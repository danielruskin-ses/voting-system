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
#include "TreeGen.shared.h"

#include <sstream>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

void printTree(Logger& logger, const Tree& tree, const std::string& soFar) {
        if(!tree.has_root()) {
                return;
        }

        std::string rootRep = "voterdeviceid= " + std::to_string(tree.root().treenode().signedrecordedballot().recordedballot().signedproposedballot().proposedballot().voterdeviceid()) + " hash=" + tree.root().hash().hash();

        logger.info(soFar + ":" + rootRep);
        printTree(logger, tree.left(), soFar + "L");
        printTree(logger, tree.right(), soFar + "R");
}

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
        auto& candidatesTwo = *(elections[1].mutable_candidates());
        candidatesTwo[0] = Candidate::default_instance();
        candidatesTwo[0].set_name("Candidate C");
        candidatesTwo[1] = Candidate::default_instance();
        candidatesTwo[1].set_name("Candidate D");

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

        logger.info("Created voter device: " + std::to_string(maxId) + "!");

        // Incr id for next time
        maxId++;
}

void castBallot(const std::vector<std::string>& cmdParts, Logger& logger, VoteServer::Stub& voteServerStub, AuditServer::Stub& auditServerStub, const std::map<int, std::pair<std::string, std::string>>& voterDevices, const std::string& voteServerPubKey, const std::string& auditServerPubKey, std::map<int, SignedRecordedBallot>& ballots) {
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
        ClientContext voteServerContext;
        Status status = voteServerStub.CastProposedBallot(&voteServerContext, signedProposedBallot, &signedRecordedBallot);
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

        // Submit recorded ballot to audit server
        SignedSubmitRecordedBallotResponse response;
        ClientContext auditServerContext;
        Status statusTwo = auditServerStub.SubmitRecordedBallot(&auditServerContext, signedRecordedBallot, &response);
        if(!statusTwo.ok()) {
                logger.error("RPC failed!");
                return;
        }


        // Validate that the returned response contains the correct signed recorded ballot
        bool sameRecorded = google::protobuf::util::MessageDifferencer::Equals(
                response.response().ballot(),
                signedRecordedBallot
        );
        if(!sameRecorded) {
                logger.error("Invalid recorded ballot!");
                return;
        }

        // Validate that the response signature is valid
        bool validSigTwo = VerifyMessage(
                response.response(),
                response.signature(),
                auditServerPubKey
        );
        if(!validSigTwo) {
                logger.error("Invalid audit server digital signature!");
                return;
        }

        ballots[voterDeviceId] = signedRecordedBallot;
        logger.info("Ballot cast successfully!");
}

void fetchFullTree(Logger& logger, VoteServer::Stub& voteServerStub, AuditServer::Stub& auditServerStub, const std::map<int, std::pair<std::string, std::string>>& voterDevices, const std::string& voteServerPublicKey, const std::string& auditServerPublicKey, const std::map<int, SignedRecordedBallot>& ballots) {
        // Fetch vote server tree
        SignedTree voteServerTree;
        ClientContext voteServerContext;
        Status voteServerStatus = voteServerStub.GetFullTree(&voteServerContext, EmptyMessage(), &voteServerTree);
        if(!voteServerStatus.ok()) {
                logger.error("RPC failed!");
                return;
        }
        bool voteServerValidSig = VerifyMessage(
                voteServerTree.tree(),
                voteServerTree.signature(),
                voteServerPublicKey
        );
        if(!voteServerValidSig) {
                logger.error("Invalid vote server digital signature!");
                return;
        }
        
        // Fetch audit server tree
        SignedTree auditServerTree;
        ClientContext auditServerContext;
        Status auditServerStatus = auditServerStub.GetFullTree(&auditServerContext, EmptyMessage(), &auditServerTree);
        if(!auditServerStatus.ok()) {
                logger.error("RPC failed!");
                return;
        }
        bool auditServerValidSig = VerifyMessage(
                auditServerTree.tree(),
                auditServerTree.signature(),
                auditServerPublicKey
        );
        if(!auditServerValidSig) {
                logger.error("Invalid audit server digital signature!");
                return;
        }

        // Verify trees are identical
        bool sameTrees = google::protobuf::util::MessageDifferencer::Equals(
                voteServerTree.tree(),
                auditServerTree.tree()
        );
        if(!sameTrees) {
                logger.error("Invalid trees!");
                return;
        }


        // Verify tree structure
        bool valid = verifyTreeStructure(voteServerTree.tree());
        if(!valid) {
                logger.error("Invalid trees!");
                return;
        }

        // Verify all of our ballots are in the tree
        for(auto& kv : ballots) {
                HashedTreeNode node = findNodeForVoterDeviceId(voteServerTree.tree(), kv.first);
                bool sameBallots = google::protobuf::util::MessageDifferencer::Equals(
                        node.treenode().signedrecordedballot(),
                        kv.second
                );

                if(!sameBallots) {
                        logger.error("Invalid trees!");
                        return;
                }
        }

        logger.info("Successfully received and validated tree!  Printing...");
        printTree(logger, voteServerTree.tree(), "");
}

void fetchPartialTree(const std::vector<std::string>& cmdParts, Logger& logger, VoteServer::Stub& voteServerStub, AuditServer::Stub& auditServerStub, const std::map<int, std::pair<std::string, std::string>>& voterDevices, const std::string& voteServerPublicKey, const std::string& auditServerPublicKey, const std::map<int, SignedRecordedBallot>& ballots) {

        // Command should have two strings:
        // 1. Command name (cast_ballot)
        // 2. Voter device ID
        if(cmdParts.size() < 2) {
                logger.error("Must pass in voter device ID");
        }
        int voterDeviceId = std::stoi(cmdParts[1]);
        IntMessage voterDeviceIdMsg;
        voterDeviceIdMsg.set_value(voterDeviceId);

        // Fetch vote server tree
        SignedTree voteServerTree;
        ClientContext voteServerContext;
        Status voteServerStatus = voteServerStub.GetPartialTree(&voteServerContext, voterDeviceIdMsg, &voteServerTree);
        if(!voteServerStatus.ok()) {
                logger.error("RPC failed!");
                return;
        }
        bool voteServerValidSig = VerifyMessage(
                voteServerTree.tree(),
                voteServerTree.signature(),
                voteServerPublicKey
        );
        if(!voteServerValidSig) {
                logger.error("Invalid vote server digital signature!");
                return;
        }
        
        // Fetch audit server tree
        SignedTree auditServerTree;
        ClientContext auditServerContext;
        Status auditServerStatus = auditServerStub.GetPartialTree(&auditServerContext, voterDeviceIdMsg, &auditServerTree);
        if(!auditServerStatus.ok()) {
                logger.error("RPC failed!");
                return;
        }
        bool auditServerValidSig = VerifyMessage(
                auditServerTree.tree(),
                auditServerTree.signature(),
                auditServerPublicKey
        );
        if(!auditServerValidSig) {
                logger.error("Invalid audit server digital signature!");
                return;
        }

        // Verify trees are identical
        bool sameTrees = google::protobuf::util::MessageDifferencer::Equals(
                voteServerTree.tree(),
                auditServerTree.tree()
        );
        if(!sameTrees) {
                logger.error("Invalid trees!");
                return;
        }

        // Verify tree structure
        bool valid = verifyTreeStructure(voteServerTree.tree());
        if(!valid) {
                logger.error("Invalid trees!");
                return;
        }

        // Verify voter device ballot is in the tree
        HashedTreeNode node = findNodeForVoterDeviceId(voteServerTree.tree(), voterDeviceId);
        bool sameBallots = google::protobuf::util::MessageDifferencer::Equals(
                node.treenode().signedrecordedballot(),
                ballots.at(voterDeviceId)
        );
        if(!sameBallots) {
                logger.error("Invalid trees!");
                return;
        }
        
        logger.info("Successfully received and validated tree!  Printing...");
        printTree(logger, voteServerTree.tree(), "");
}

int main(int argc, char* argv[]) {
        Logger logger;

        // Wipe existing databases
        std::remove("../vote_server/database/database.db");
        std::remove("../audit_server/database/database.db");

        // Connect to vote server
        VoteServerDatabase voteServerDatabase("../vote_server/database", logger);
        std::shared_ptr<Channel> voteServerChannel(grpc::CreateChannel("0.0.0.0:8001", grpc::InsecureChannelCredentials()));
        std::unique_ptr<VoteServer::Stub> voteServerStub(VoteServer::NewStub(voteServerChannel));

        // Connect to audit server
        AuditServerDatabase auditServerDatabase("../audit_server/database", logger);
        std::shared_ptr<Channel> auditServerChannel(grpc::CreateChannel("0.0.0.0:8002", grpc::InsecureChannelCredentials()));
        std::unique_ptr<AuditServer::Stub> auditServerStub(AuditServer::NewStub(auditServerChannel));

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
                        castBallot(cmdParts, logger, *voteServerStub, *auditServerStub, voterDevices, auditServerDatabase.fetchVoteServerPublicKey(), auditServerDatabase.fetchAuditServerPublicKey(), ballots);
                } else if(cmdParts[0] == "fetch_full_tree") {
                        fetchFullTree(logger, *voteServerStub, *auditServerStub, voterDevices, auditServerDatabase.fetchVoteServerPublicKey(), auditServerDatabase.fetchAuditServerPublicKey(), ballots);
                } else if(cmdParts[0] == "fetch_partial_tree") {
                        fetchPartialTree(cmdParts, logger, *voteServerStub, *auditServerStub, voterDevices, auditServerDatabase.fetchVoteServerPublicKey(), auditServerDatabase.fetchAuditServerPublicKey(), ballots);
                } else {
                        logger.error("Invalid command!");
                }
        }
}

