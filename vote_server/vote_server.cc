#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "AsyncWork.h"
#include "Database.h"
#include "Logger.shared.h"
#include "Crypto.shared.h"
#include "TreeGen.shared.h"
#include "vote_server.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

class VoteServerImpl final : public VoteServer::Service {
public:
        explicit VoteServerImpl(const std::string& databasePath) : _logger(), _database(databasePath, _logger), _asyncWork(_logger, _database) {
        }

        Status GetElectionMetadata(ServerContext* context, const Empty* empty, ElectionMetadata* electionMetadata) override {
                _logger.info("GetElectionMetadata");
                electionMetadata->CopyFrom(_database.fetchElectionMetadata());

                std::string serialized;
                electionMetadata->SerializeToString(&serialized);
                std::string signature = SignMessage(
                        serialized,
                        _database.fetchVoteServerPrivateKey()
                );
                electionMetadata->mutable_signature()->set_signature(std::move(signature));

                _logger.info("GetElectionMetadata: OK");
                return Status::OK;
        }

        Status CastProposedBallot(ServerContext* context, const ProposedBallot* proposedBallot, RecordedBallot* recordedBallot) override {
                _logger.info("CastProposedBallot");

                // Fetch election metadata
                ElectionMetadata metadata;
                metadata.CopyFrom(_database.fetchElectionMetadata());

                // Perform validations
                // 1. Are polls currently open?
                std::time_t currentTime = std::time(0);
                int startEpoch = metadata.electionstart().epoch();
                int endEpoch = metadata.electionend().epoch();
                if(currentTime < startEpoch || currentTime > endEpoch) {
                        _logger.info("CastProposedBallot: ERROR, polls are closed");
                        throw std::runtime_error("Polls are closed!");
                }

                // 2. Is cast-at timestamp within polling hours?
                int castAtTimestamp = proposedBallot->castat().epoch();
                if(castAtTimestamp < startEpoch || castAtTimestamp > endEpoch) {
                        _logger.info("CastProposedBallot: ERROR, ballot cast-at timestamp outside of polling hours");
                        throw std::runtime_error("Cast-at timestamp outside of polling hours!");
                }
        
                // 3. Does proposed ballot have valid candidate choices for every valid election?
                for(auto it = metadata.elections().cbegin(); it != metadata.elections().cend(); it++) {
                        int electionId = it->first;
                        const Election& election = it->second;

                        if(proposedBallot->candidatechoices().find(electionId) == proposedBallot->candidatechoices().end()) {
                                _logger.info("CastProposedBallot: ERROR, ballot does not contain vote for election");
                                throw std::runtime_error("Ballot does not contain vote for election!");
                        }
                        int candidateChoice = proposedBallot->candidatechoices().at(electionId);

                        if(election.candidates().find(candidateChoice) == election.candidates().end()) {
                                _logger.info("CastProposedBallot: ERROR, invalid candidate choice for election");
                                throw std::runtime_error("Invalid candidate choice for election!");
                        }
                }

                // 4. Does proposed ballot have any extra votes?
                if(proposedBallot->candidatechoices().size() != metadata.elections().size()) {
                        _logger.info("CastProposedBallot: ERROR, extra votes");
                        throw std::runtime_error("Extra votes!");
                }
     
                // 4. Is proposed ballot signature valid?
                ProposedBallot proposedBallotWithoutSig = *proposedBallot;
                proposedBallotWithoutSig.clear_voterdevicesignature();
                std::string proposedBallotWithoutSigSerialized;
                proposedBallotWithoutSig.SerializeToString(&proposedBallotWithoutSigSerialized);
                bool validSig = VerifyMessage(
                        proposedBallotWithoutSigSerialized,
                        proposedBallot->voterdevicesignature().signature(),
                        _database.fetchVoterDevicePublicKey(proposedBallot->voterdeviceid())
                );
                if(!validSig) {
                        _logger.info("CastProposedBallot: ERROR, invalid digital signature");
                        throw std::runtime_error("Invalid digital signature!");
                }

                // All validations passed, generate recorded ballot
                *(recordedBallot->mutable_proposedballot()) = *proposedBallot;
                std::string recordedBallotSerialized;
                recordedBallot->SerializeToString(&recordedBallotSerialized);
                std::string signature = SignMessage(
                        recordedBallotSerialized,
                        _database.fetchVoteServerPrivateKey()
                );
                recordedBallot->mutable_voteserversignature()->set_signature(std::move(signature));

                // Save recorded ballot to database
                _database.saveRecordedBallot(proposedBallot->voterdeviceid(), *recordedBallot);
                
                _logger.info("CastProposedBallot: OK");
                return Status::OK;
        }

        Status GetFullTree(ServerContext* context, const Empty* empty, SignedTree* signedTree) override {
                _logger.info("GetFullTree");

                signedTree->CopyFrom(_database.fetchSignedTree());

                _logger.info("GetFullTree: OK");
                return Status::OK;
        }

        Status GetPartialTree(ServerContext* context, const IntMessage* voterDeviceId, SignedTree* signedTree) override {
                _logger.info("GetPartialTree");

                // Fetch full tree, generate partial tree from full tree
                SignedTree fullTree = _database.fetchSignedTree();
                getPartialTree(fullTree.tree(), voterDeviceId->value(), signedTree->mutable_tree());

                // Sign partial tree
                std::string signedTreeSerialized;
                signedTree->SerializeToString(&signedTreeSerialized);
                std::string signature = SignMessage(
                        signedTreeSerialized,
                        _database.fetchVoteServerPrivateKey()
                );
                signedTree->mutable_signature()->set_signature(std::move(signature));

                _logger.info("GetPartialTree: OK");
                return Status::OK;
        }

private:
        AsyncWork _asyncWork;
        Logger _logger;
        Database _database;
};

void RunServer(const std::string& databasePath) {
        std::string serverAddress("0.0.0.0:8001");
        VoteServerImpl voteServer(databasePath);

        ServerBuilder builder;
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(&voteServer);

        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Server listening on: " << serverAddress << std::endl;
        server->Wait();
}

int main(int argc, char** argv) {
        RunServer("database/");
        return 0;
}
