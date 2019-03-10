#include "Database.h"

#include <set>
#include <fstream>
#include <dirent.h>
#include "sqlpp11/postgresql/connection_config.h"
#include "sqlpp11/custom_query.h"
#include "sqlpp11/verbatim.h"

Database::Database(const std::string& user, const std::string& password, const std::string& host, const std::string& db_name, const std::string& migrations) : _user(user), _password(password), _host(host), _db_name(db_name), _migrations(migrations) {
        auto config = std::make_shared<sqlpp::postgresql::connection_config>();
        config->host = host;
        config->user = user;
        config->password = password;
        config->dbname = db_name;
        
        _db = sqlpp::postgresql::connection(config);
}

void Database::migrate() {
        // Get files, sort alphabetically
        DIR *dir = opendir(_migrations.c_str());
        if(dir == NULL) {
                throw std::runtime_error("Failed to open migrations directory!");
        }
        std::set<std::string> files;
        struct dirent *ent;
        while((ent = readdir(dir)) != NULL) {
                files.insert(std::string(ent->d_name));
        }
        closedir(dir);
        
        // Run each migration
        for(const auto& file : files) {
                // Read migration into string
                std::ifstream fileObj(file);
                std::stringstream buf;
                buf << fileObj.rdbuf();
        
                // Execute migration
                _db.execute(buf.str());
        }
}
