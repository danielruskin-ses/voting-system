#pragma once

#include <string>

class Config {
public:
        Config(const std::string& db_user, const std::string& db_pass, const std::string& db_host, const std::string& db_name, const std::string& db_migrations, const std::string& privkey_hex) : _db_user(db_user), _db_pass(db_pass), _db_host(db_host), _db_name(db_name), _db_migrations(db_migrations), _privkey_hex(privkey_hex) { }

        const std::string& dbUser() const { return _db_user; }
        const std::string& dbPass() const { return _db_pass; }
        const std::string& dbHost() const { return _db_host; }
        const std::string& dbName() const { return _db_name; }
        const std::string& dbMigrations() const { return _db_migrations; }

        const std::string& privkeyHex() const { return _privkey_hex; }

private:
        std::string _db_user;
        std::string _db_pass;
        std::string _db_host;
        std::string _db_name;
        std::string _db_migrations;
        
        std::string _privkey_hex;
};
