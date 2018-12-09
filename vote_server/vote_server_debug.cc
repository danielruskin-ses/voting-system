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
        // Create some dummy data
        if(argc >= 2 && strcmp(argv[1], "create_dummy") == 0) {
                std::cout << "got to 1" << std::endl;
                Logger l;
                Database db("database/", l);
                
                // Create election metadata
                ElectionMetadata em;
                em.mutable_electionstart()->set_epoch(100);
                em.mutable_electionend()->set_epoch(200);
                em.add_elections();
                em.mutable_elections(0)->set_description("Hello");
                em.mutable_elections(0)->add_candidateoptions();
                em.mutable_elections(0)->mutable_candidateoptions(0)->set_name("Daniel");
                em.mutable_elections(0)->mutable_candidateoptions(0)->set_id(0);
                db.saveElectionMetadata(1, em);
        }
        
        // Connect to vote server
        std::shared_ptr<Channel> channel(grpc::CreateChannel("0.0.0.0:8001", grpc::InsecureChannelCredentials()));
        std::unique_ptr<VoteServer::Stub> stub(VoteServer::NewStub(channel));

        ClientContext context;
        ElectionMetadata em;
        Status status = stub->GetElectionMetadata(&context, Empty(), &em);
        if(!status.ok()) {
                throw std::runtime_error("RPC failed!");
        }

        // Test election metadata
        std::cout << "Start Epoch: " << em.electionstart().epoch() << std::endl;
        std::cout << "End Epoch: " << em.electionend().epoch() << std::endl;
        std::cout << "Eletion 0, Desc: " << em.elections(0).description() << std::endl;
        std::cout << "Eletion 0, Candidate 0 Name: " << em.elections(0).candidateoptions(0).name() << std::endl;
        std::cout << "Eletion 0, Candidate 0 ID: " << em.elections(0).candidateoptions(0).id() << std::endl;

        return 0;
}
