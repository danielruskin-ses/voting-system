#pragma once 

#include "AsyncWork.shared.h"
#include "AuditServerDatabase.h"
#include "audit_server.grpc.pb.h"

class AuditServerAsyncWork : public AsyncWork {
public:
        explicit AuditServerAsyncWork(const Logger& logger, AuditServerDatabase& database);
        ~AuditServerAsyncWork() = default;

        void loopInner() override;
private:
        AuditServerDatabase& _database;

        void fetchSignedTreeIfNeeded();
};
