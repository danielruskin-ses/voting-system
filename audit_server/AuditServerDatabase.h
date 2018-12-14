#pragma once 

#include <sqlite3.h>
#include <mutex>

#include "Logger.shared.h"
#include "Database.shared.h"
#include "audit_server.grpc.pb.h"

class AuditServerDatabase : public Database {
public:
        explicit AuditServerDatabase(const std::string& databasePath, const Logger& logger);
};
