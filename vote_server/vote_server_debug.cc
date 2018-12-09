#include <cstring>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "Database.h"
#include "Crypto.shared.h"
#include "vote_server.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

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
                em.mutable_electionend()->set_epoch(std::time(0) + 100000);
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

                std::cout << "Election 0, Desc: " << em.elections().size() << std::endl;
                em.CopyFrom(db.fetchElectionMetadata());
                std::cout << "Election 0, Desc: " << em.elections().size() << std::endl;
        } else if(command == "create_voter_device") {
                // Create voter device crypto keys
                std::string pubKey;
                std::string privKey;
                GenerateKeyPair(pubKey, privKey);

                // Persist voter device
                db.saveVoterDevicePublicKey(1, pubKey);

                logger.info("Voter device saved!");
                logger.info("Voter device pubkey: " + pubKey);
                logger.info("Voter device privkey: " + privKey);
        } else if(command == "fetch_election_metadata") {
                ElectionMetadata em;
                Status status = stub->GetElectionMetadata(&context, Empty(), &em);
                if(!status.ok()) {
                        throw std::runtime_error("RPC failed!");
                }

                std::cout << "Start Epoch: " << em.electionstart().epoch() << std::endl;
                std::cout << "End Epoch: " << em.electionend().epoch() << std::endl;
                std::cout << "Election 0, Desc: " << em.elections().at(0).description() << std::endl;
                std::cout << "Election 0, Candidate 0 Name: " << em.elections().at(0).candidates().at(0).name() << std::endl;
                std::cout << "Election 0, Candidate 1 Name: " << em.elections().at(0).candidates().at(1).name() << std::endl;

                ElectionMetadata emWithoutSig = em;
                emWithoutSig.clear_signature();
                std::string serializedWithoutSig;
                emWithoutSig.SerializeToString(&serializedWithoutSig);
                bool validSig = VerifyMessage(
                        serializedWithoutSig,
                        em.signature().signature(),
                        db.fetchVoteServerPublicKey()
                );
                std::cout << "Election metadata valid signature? " << (validSig ? "YES" : "NO") << std::endl;

        } else {
                throw std::runtime_error("Invalid command!");
        }
        
        return 0;
}
