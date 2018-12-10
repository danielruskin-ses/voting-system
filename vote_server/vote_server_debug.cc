#include <cstring>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "Database.h"
#include "TreeGen.shared.h"
#include "Crypto.shared.h"
#include "vote_server.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

void printTree(const Logger& logger, const Tree& tree, const std::string& dots) {
        if(tree.has_root()) {
                logger.info(dots + "Node hash=" + tree.root().hash().hash() + " voterdeviceid=" + std::to_string(tree.root().recordedballot().proposedballot().voterdeviceid()));
        }

        if(tree.has_left()) {
                printTree(logger, tree.left(), dots + ".");
        }
        if(tree.has_right()) {
                printTree(logger, tree.right(), dots + ".");
        }
}

int main(int argc, char** argv) {
        // Parse command
        if(argc < 2) {
                throw std::runtime_error("Must pass a command!");
        }
        std::string command(argv[1]);

        // Connect to db
        Logger logger;
        Database db("database/", logger);

        // Connect to vote server
        std::shared_ptr<Channel> channel(grpc::CreateChannel("0.0.0.0:8001", grpc::InsecureChannelCredentials()));
        std::unique_ptr<VoteServer::Stub> stub(VoteServer::NewStub(channel));
        ClientContext context;

        if(command == "create_config") {
                // Create ElectionMetadata with a single election
                ElectionMetadata em;
                em.mutable_electionstart()->set_epoch(std::time(0));
                em.mutable_electionend()->set_epoch(std::time(0) + 10);
                auto& elections = *em.mutable_elections();
                elections[0] = Election::default_instance();
                elections[0].set_description("President");
                auto& candidates = *(elections[0].mutable_candidates());
                candidates[0] = Candidate::default_instance();
                candidates[0].set_name("John Smith");
                candidates[1] = Candidate::default_instance();
                candidates[1].set_name("Betty Johnson");

                // Create vote server crypto keys
                std::string pubKey;
                std::string privKey;
                GenerateKeyPair(pubKey, privKey);

                // Persist config
                db.saveConfig(1, em, pubKey, privKey);

                logger.info("Election config saved!");
                logger.info("Vote server pubkey: " + pubKey);
                logger.info("Vote server privkey: " + privKey);
        } else if(command == "create_voter_device") {
                // Get extra params
                if(argc < 3) {
                        throw std::runtime_error("Must pass voter device ID!");
                }
                int voterDeviceId = atoi(argv[2]);

                // Create voter device crypto keys
                std::string pubKey;
                std::string privKey;
                GenerateKeyPair(pubKey, privKey);

                // Persist voter device
                db.saveVoterDevicePublicKey(voterDeviceId, pubKey);

                logger.info("Voter device saved!");
                logger.info("Voter device ID: " + std::to_string(voterDeviceId));
                logger.info("Voter device pubkey: " + pubKey);
                logger.info("Voter device privkey: " + privKey);
        } else if(command == "fetch_election_metadata") {
                ElectionMetadata em;
                Status status = stub->GetElectionMetadata(&context, Empty(), &em);
                if(!status.ok()) {
                        throw std::runtime_error("RPC failed!");
                }

                logger.info("Start Epoch: " + std::to_string(em.electionstart().epoch()));
                logger.info("End Epoch: " + std::to_string(em.electionend().epoch()));
                logger.info("Election 0, Desc: " + em.elections().at(0).description());
                logger.info("Election 0, Candidate 0 Name: " + em.elections().at(0).candidates().at(0).name());
                logger.info("Election 0, Candidate 1 Name: " + em.elections().at(0).candidates().at(1).name());

                ElectionMetadata emWithoutSig = em;
                emWithoutSig.clear_signature();
                std::string serializedWithoutSig;
                emWithoutSig.SerializeToString(&serializedWithoutSig);
                bool validSig = VerifyMessage(
                        serializedWithoutSig,
                        em.signature().signature(),
                        db.fetchVoteServerPublicKey()
                );
                logger.info("Election metadata valid signature? " + std::to_string(validSig));

        } else if(command == "cast_proposed_ballot") {
                // Get extra params
                if(argc < 4) {
                        throw std::runtime_error("Must pass voter device ID and privkey!  Generate with create_voter_device if needed.");
                }
                int voterDeviceId = atoi(argv[2]);
                std::string privKey(argv[3]);

                // Generate and sign proposed ballot
                ProposedBallot proposedBallot;
                proposedBallot.set_voterdeviceid(voterDeviceId);
                proposedBallot.mutable_castat()->set_epoch(std::time(0));
                auto& candidateChoices = *(proposedBallot.mutable_candidatechoices());
                candidateChoices[0] = 1;
                std::string proposedBallotSerialized;
                proposedBallot.SerializeToString(&proposedBallotSerialized);
                std::string signature = SignMessage(
                        proposedBallotSerialized,
                        privKey
                );
                proposedBallot.mutable_voterdevicesignature()->set_signature(std::move(signature));

                // Transmit proposed ballot and get recorded ballot
                RecordedBallot recordedBallot;
                Status status = stub->CastProposedBallot(&context, proposedBallot, &recordedBallot);
                if(!status.ok()) {
                        throw std::runtime_error("RPC failed!");
                }

                std::string proposedBallotSerializedWithSig;
                std::string enclosedProposedBallotSerialized;
                proposedBallot.SerializeToString(&proposedBallotSerializedWithSig);
                recordedBallot.proposedballot().SerializeToString(&enclosedProposedBallotSerialized);
                logger.info("Recorded Ballot Enclosed Proposed Ballot == Submitted Proposed Ballot? " + std::to_string(proposedBallotSerializedWithSig == enclosedProposedBallotSerialized));

                RecordedBallot rbWithoutSig = recordedBallot;
                rbWithoutSig.clear_voteserversignature();
                std::string serializedWithoutSig;
                rbWithoutSig.SerializeToString(&serializedWithoutSig);
                bool validSig = VerifyMessage(
                        serializedWithoutSig,
                        recordedBallot.voteserversignature().signature(),
                        db.fetchVoteServerPublicKey()
                );
                logger.info("Recorded ballot valid signature? " + std::to_string(validSig));
                
        } else if(command == "fetch_full_tree") {
                SignedTree st;

                Status status = stub->GetFullTree(&context, Empty(), &st);
                if(!status.ok()) {
                        throw std::runtime_error("RPC failed!");
                }

                printTree(logger, st.tree(), "");

                SignedTree stWithoutSig = st;
                stWithoutSig.clear_signature();
                std::string serializedWithoutSig;
                stWithoutSig.SerializeToString(&serializedWithoutSig);
                bool validSig = VerifyMessage(
                        serializedWithoutSig,
                        st.signature().signature(),
                        db.fetchVoteServerPublicKey()
                );
                logger.info("Signed tree valid signature? " + std::to_string(validSig));

        } else if(command == "fetch_partial_tree") {
                // Get extra param
                if(argc < 2) {
                        throw std::runtime_error("Must pass voter device ID!");
                }
                int voterDeviceId = atoi(argv[2]);

                IntMessage im;
                im.set_value(voterDeviceId);
                SignedTree st;

                Status status = stub->GetPartialTree(&context, im, &st);
                if(!status.ok()) {
                        throw std::runtime_error("RPC failed!");
                }

                printTree(logger, st.tree(), "");

                SignedTree stWithoutSig = st;
                stWithoutSig.clear_signature();
                std::string serializedWithoutSig;
                stWithoutSig.SerializeToString(&serializedWithoutSig);
                bool validSig = VerifyMessage(
                        serializedWithoutSig,
                        st.signature().signature(),
                        db.fetchVoteServerPublicKey()
                );
                logger.info("Signed tree valid signature? " + std::to_string(validSig));

        } else if(command == "execute_query") {
                // Get extra params
                if(argc < 2) {
                        throw std::runtime_error("Must pass query!");
                }

                std::string query(argv[2]);
                db.executeQuery(query);
        } else {
                throw std::runtime_error("Invalid command!");
        }
        
        return 0;
}
