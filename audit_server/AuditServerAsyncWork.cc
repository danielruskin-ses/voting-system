#include "AuditServerAsyncWork.h"
#include "Crypto.shared.h"
#include "TreeGen.shared.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "vote_server.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

AuditServerAsyncWork::AuditServerAsyncWork(const Logger& logger, AuditServerDatabase& database) : _database(database), AsyncWork(logger) {
}

void AuditServerAsyncWork::loopInner() {
        fetchSignedTreeIfNeeded();
}

void AuditServerAsyncWork::fetchSignedTreeIfNeeded() {
        // Only fetch a single time
        if(_database.isSignedTreeReceived()) {
                return;
        }

        // Only fetch once polls close
        ElectionMetadata metadata = _database.fetchElectionMetadata();
        if(std::time(0) <= metadata.electionend().epoch()) {
                return;
        }

        // Connect to vote server
        std::shared_ptr<Channel> channel(grpc::CreateChannel("0.0.0.0:8001", grpc::InsecureChannelCredentials()));
        std::unique_ptr<VoteServer::Stub> stub(VoteServer::NewStub(channel));
        ClientContext context;

        // Attempt to fetch tree
        SignedTree signedTree;
        Status status = stub->GetFullTree(&context, EmptyMessage(), &signedTree);
        if(!status.ok()) {
                _logger.error("ALARM: Failed to fetch signed tree!");
                return;
        }

        // Validate signed tree signature
        bool validSig = VerifyMessage(
                signedTree.tree(),
                signedTree.signature(),
                _database.fetchVoteServerPublicKey()
        );
        if(!validSig) {
                _logger.error("ALARM: Failed to validate signed tree signature!");
                return;
        }

        // Validate that tree is correct
        // (i.e. retrieved tree matches the tree we would have generated,
        //  and thus contains all ballots)
        Tree comparableTree;
        treeGen(_database.fetchSignedRecordedBallotsSorted(), &comparableTree);
        bool sameTrees = google::protobuf::util::MessageDifferencer::Equals(signedTree.tree(), comparableTree);
        if(!sameTrees) {
                _logger.error("ALARM: Failed to validate signed tree!");
                return;
        }

        // Save tree
        _database.saveSignedTree(1, signedTree);
}
