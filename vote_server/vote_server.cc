#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <grpcpp/security/server_credentials.h>

#include "Database.h"
#include "Logger.shared.h"
#include "Crypto.shared.h"
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
        explicit VoteServerImpl(const std::string& databasePath) : _logger(), _database(databasePath, _logger) {
        }

        Status GetElectionMetadata(ServerContext* context, const Empty* empty, ElectionMetadata* electionMetadata) override {
                _logger.info("Election Metadata Fetch");
                electionMetadata->CopyFrom(_database.fetchElectionMetadata());

                std::string serialized;
                electionMetadata->SerializeToString(&serialized);
                std::string signature = SignMessage(
                        serialized,
                        _database.fetchVoteServerPrivateKey()
                );
                electionMetadata->set_allocated_signature(Signature::default_instance().New());
                electionMetadata->mutable_signature()->set_signature(std::move(signature));
                        
                _logger.info("Election Metadata Fetch OK");
                return Status::OK;
        }

        Status CastProposedBallot(ServerContext* context, const ProposedBallot* proposedBallot, RecordedBallot* recordedBallot) override {
                return Status::OK;
        }
private:
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
