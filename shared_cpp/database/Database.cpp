#include "Database.h"

#include <set>
#include <fstream>
#include <experimental/filesystem>
#include <memory>

Database::Database(const std::string& user, const std::string& password, const std::string& host, const std::string& db_name, const std::string& migrations) : _connStr("dbname=" + db_name + " user=" + user + " password=" + password + " host=" + host), _migrations(migrations) {
}


std::unique_ptr<pqxx::connection> Database::getConnection() const {
        return std::make_unique<pqxx::connection>(_connStr);
}

void Database::migrate() {
        std::unique_ptr<pqxx::connection> conn = getConnection();
        pqxx::work txn(*conn);

        // Acquire lock on migrations table
        txn.exec("LOCK migrations IN ACCESS EXCLUSIVE MODE");

        // Get files, sort alphabetically
        std::set<std::string> files;
        for(const auto& entry : std::experimental::filesystem::directory_iterator(_migrations)) {
                files.insert(std::string(entry.path()));
        }
        
        // Run each migration
        for(const auto& file : files) {
                // Skip this migration if already performed
                pqxx::result r = txn.exec(
                        "SELECT id"
                        "FROM migrations"
                        "WHERE name = " + txn.quote(file));
                if(r.size() != 0) {
                        continue;
                }
                
                // Read migration into string and execute
                std::ifstream fileObj(file);
                std::stringstream buf;
                buf << fileObj.rdbuf();
                txn.exec(buf.str());
        }

        // Commit our changes to the db
        txn.commit();
}
