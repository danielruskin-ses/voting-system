#include "AuditServerAsyncWork.h"
#include "Crypto.shared.h"
#include "TreeGen.shared.h"

AuditServerAsyncWork::AuditServerAsyncWork(const Logger& logger, AuditServerDatabase& database) : _database(database), AsyncWork(logger) {
}

void AuditServerAsyncWork::loopInner() {
        // TODO
}
