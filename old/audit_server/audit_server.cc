#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "AuditServerAsyncWork.h"
#include "AuditServerDatabase.h"
#include "Logger.shared.h"
#include "Crypto.shared.h"
#include "TreeGen.shared.h"
#include "vote_server.grpc.pb.h"
#include "audit_server.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

class AuditServerImpl final : public AuditServer::Service {
public:
        explicit AuditServerImpl(const std::string& databasePath) : _logger(), _database(databasePath, _logger), _asyncWork(_logger, _database) {
        }

        Status GetElectionMetadata(ServerContext* context, const EmptyMessage* empty, SignedElectionMetadata* signedElectionMetadata) override {
                _logger.info("GetElectionMetadata");

                signedElectionMetadata->mutable_electionmetadata()->CopyFrom(_database.fetchElectionMetadata());

                SignMessage(
                        signedElectionMetadata->electionmetadata(),
                        signedElectionMetadata->mutable_signature(),
                        _database.fetchAuditServerPrivateKey()
                );
                        
                _logger.info("GetElectionMetadata: OK");
                return Status::OK;
        }

        Status SubmitRecordedBallot(ServerContext* context, const SignedRecordedBallot* signedRecordedBallot, SignedSubmitRecordedBallotResponse* response) override {
                _logger.info("SubmitRecordedBallot");

                // Fetch election metadata
                ElectionMetadata metadata;
                metadata.CopyFrom(_database.fetchElectionMetadata());

                // Fetch recorded ballot and proposed ballot
                const RecordedBallot& recordedBallot = signedRecordedBallot->recordedballot();
                const SignedProposedBallot& signedProposedBallot = recordedBallot.signedproposedballot();
                const ProposedBallot& proposedBallot = signedProposedBallot.proposedballot();

                // Validate digital signatures
                bool validSignatures = 
                        VerifyMessage(
                                proposedBallot,
                                signedProposedBallot.signature(),
                                _database.fetchVoterDevicePublicKey(proposedBallot.voterdeviceid())
                        ) && VerifyMessage(
                                recordedBallot,
                                signedRecordedBallot->signature(),
                                _database.fetchVoteServerPublicKey()
                        );
                if(!validSignatures) {
                        _logger.info("SubmitRecordedBallot: ERROR, invalid digital signature");
                        throw std::runtime_error("Invalid digital signature!");
                }

                // Is cast-at timestamp within polling hours?
                int startEpoch = metadata.electionstart().epoch();
                int endEpoch = metadata.electionend().epoch();
                int castAtTimestamp = proposedBallot.castat().epoch();
                if(castAtTimestamp < startEpoch || castAtTimestamp > endEpoch) {
                        _logger.info("SubmitRecordedBallot: ERROR, ballot cast-at timestamp outside of polling hours");
                        throw std::runtime_error("Cast-at timestamp outside of polling hours!");
                }

                // Does proposed ballot have valid candidate choices for every valid election?
                for(auto it = metadata.elections().cbegin(); it != metadata.elections().cend(); it++) {
                        int electionId = it->first;
                        const Election& election = it->second;

                        if(proposedBallot.candidatechoices().find(electionId) == proposedBallot.candidatechoices().end()) {
                                _logger.info("SubmitRecordedBallot: ERROR, ballot does not contain vote for election");
                                throw std::runtime_error("Ballot does not contain vote for election!");
                        }
                        int candidateChoice = proposedBallot.candidatechoices().at(electionId);

                        if(election.candidates().find(candidateChoice) == election.candidates().end()) {
                                _logger.info("SubmitRecordedBallot: ERROR, invalid candidate choice for election");
                                throw std::runtime_error("Invalid candidate choice for election!");
                        }
                }

                // Does proposed ballot have any extra votes?
                if(proposedBallot.candidatechoices().size() != metadata.elections().size()) {
                        _logger.info("SubmitRecordedBallot: ERROR, extra votes");
                        throw std::runtime_error("Extra votes!");
                }

                // Recorded ballot is valid, save
                _database.saveSignedRecordedBallot(proposedBallot.voterdeviceid(), *signedRecordedBallot);

                response->mutable_response()->mutable_ballot()->CopyFrom(*signedRecordedBallot);
                response->mutable_response()->mutable_receivedat()->set_epoch(std::time(0));
                SignMessage(
                        response->response(),
                        response->mutable_signature(),
                        _database.fetchAuditServerPrivateKey()
                );

                _logger.info("SubmitRecordedBallot OK");
                return Status::OK;
        }

        Status GetFullTree(ServerContext* context, const EmptyMessage* empty, SignedTree* signedTree) override {
                _logger.info("GetFullTree");

                signedTree->CopyFrom(_database.fetchSignedTree());

                // Re-sign as audit server (db stores vote server signed tree)
                SignMessage(
                        signedTree->tree(),
                        signedTree->mutable_signature(),
                        _database.fetchAuditServerPrivateKey()
                );

                _logger.info("GetFullTree: OK");
                return Status::OK;
        }

        Status GetPartialTree(ServerContext* context, const IntMessage* voterDeviceId, SignedTree* signedTree) override {
                _logger.info("GetPartialTree");

                // Fetch full tree, generate partial tree from full tree
                SignedTree fullTree = _database.fetchSignedTree();
                getPartialTree(fullTree.tree(), voterDeviceId->value(), signedTree->mutable_tree());

                // Sign partial tree
                SignMessage(
                        signedTree->tree(),
                        signedTree->mutable_signature(),
                        _database.fetchAuditServerPrivateKey()
                );

                _logger.info("GetPartialTree: OK");
                return Status::OK;
        }

private:
        AuditServerAsyncWork _asyncWork;
        Logger _logger;
        AuditServerDatabase _database;
};

void RunServer(const std::string& databasePath) {
        std::string serverAddress("0.0.0.0:8002");
        AuditServerImpl auditServer(databasePath);

        ServerBuilder builder;
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(&auditServer);

        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Server listening on: " << serverAddress << std::endl;
        server->Wait();
}

int main(int argc, char** argv) {
        RunServer("database/");
        return 0;
}
