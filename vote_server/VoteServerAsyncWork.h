#pragma once 

#include "AsyncWork.shared.h"
#include "VoteServerDatabase.h"
#include "vote_server.grpc.pb.h"

class VoteServerAsyncWork : public AsyncWork {
public:
        explicit VoteServerAsyncWork(const Logger& logger, VoteServerDatabase& database);
        ~VoteServerAsyncWork() = default;

        void loopInner() override;
private:
        VoteServerDatabase& _database;
};
