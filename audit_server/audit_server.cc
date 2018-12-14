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
        }

        Status SubmitRecordedBallot(ServerContext* context, const SignedRecordedBallot* signedRecordedBallot, SignedSubmitRecordedBallotResponse* response) override {
        }

        Status GetFullTree(ServerContext* context, const EmptyMessage* empty, SignedTree* signedTree) override {
        }

        Status GetPartialTree(ServerContext* context, const IntMessage* voterDeviceId, SignedTree* signedTree) override {
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
