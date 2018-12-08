#include <sqlite3.h>
#include <iostream>
#include <fstream>

#include "vote_server.grpc.pb.h"

class Database {
public:
        explicit Database(const std::string& databasePath) {
                // Make sqlite3 threadsafe
                sqlite3_config(SQLITE_CONFIG_SERIALIZED);

                // Open database
                int dbOpenResp = sqlite3_open((databasePath + "/vote_server.db").c_str(), &database);
                if(dbOpenResp) {
                        throw("Error opening database!");
                }

                // Load database setup query
                std::string setupQuery;
                std::ifstream setupFile(databasePath + "/database_setup.sql");
                if(setupFile.is_open()) {
                        std::string line;
                        while(std::getline(setupFile, line)) {
                                setupQuery += line + "\n";
                        }
                } else {
                        throw("Error opening database setup file!");
                }

                // Run database setup query
                char* dbSetupErrMsg = NULL;
                int dbSetupResp = sqlite3_exec(database, setupQuery.c_str(), NULL, 0, &dbSetupErrMsg);
                if(dbSetupResp != SQLITE_OK) {
                        std::cout << "SQLITE ERROR: " << dbSetupErrMsg << std::endl;
                        sqlite3_free(dbSetupErrMsg);
                        throw("Error running database setup query!");
                }
        }

        ElectionMetadata fetchElectionMetadata();
        std::string fetchVoterDevicePublicKey(int voterDeviceId);
        RecordedBallot fetchRecordedBallot(int voterDeviceId);
        void saveRecordedBallot(const RecordedBallot& recordedBallot);

        ~Database() {
                sqlite3_close(database);
        }

private:
        sqlite3* database;
};
