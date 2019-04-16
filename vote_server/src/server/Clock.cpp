#include "Clock.h"
#include "TallyGenerator.h"

void Clock::tick() {
        generateTallies();
}

void Clock::generateTallies() {
        _logger->info("Generating tallies...");

        pqxx::work txn(*_dbConn);

        // Get completed elections with no tallies
        pqxx::result r = txn.exec(
                "SELECT e.id"
                " FROM elections e"
                " LEFT JOIN tallies t ON e.id = t.election_id"
                " WHERE t.id IS NULL");

        // Generate tallies for each election
        for(int i = 0; i < r.size(); i++) {
                generateTally(r[i][0].as<int>(), txn, *_logger);
        }

        txn.commit();
}
