#pragma once

#include "../Config.h"
#include "shared_cpp/logger/Logger.h"
#include "shared_cpp/database/Database.h"

#include "gen_c/pb/shared.pb.h"

#define NULL_WRITE_IN_VALUE 0

class Client {
public:
        Client(std::shared_ptr<const Config> config, std::shared_ptr<Logger> logger) : _database(config->dbUser(), config->dbPass(), config->dbHost(), config->dbPort(), config->dbName(), config->dbMigrations()), _config(config), _logger(logger), _clientPubKey(_config->clientPubKey()) { }

        void start();
private:
        int newConn();

        std::shared_ptr<Logger> _logger;
        std::shared_ptr<const Config> _config;
        // Copy this over so we have a non-const version of it
        // Mutable b/c nanopb requires a void* ptr, even though it doesn't change it
        mutable std::vector<BYTE_T> _clientPubKey;

        Database _database;

        bool sendCommand(int sock, CommandType commandType, std::vector<BYTE_T>& data);
        std::tuple<bool, ResponseType, std::vector<BYTE_T>> getResponse(int sock);
	bool validateWriteInTally(int electionId, const std::vector<std::tuple<WriteInBallotEntry, std::vector<BYTE_T>, std::vector<BYTE_T>>>& writeInBallotEntryArr, const std::vector<std::tuple<WriteInTallyEntry, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>>>& writeInTallyEntryArr);
        std::tuple<bool, std::vector<Election>, std::vector<std::vector<int>>, std::vector<std::vector<std::tuple<Candidate, std::string, std::string>>>, std::vector<std::vector<std::tuple<TallyEntry, std::vector<BYTE_T>, std::vector<BYTE_T>>>>> getElections(bool output);
        void castBallot();
        std::tuple<bool, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>> createKeypair();
        std::tuple<std::string, std::string, std::string> createPaillierKeypair();

        bool verifyTallyEncryption(int electionId, int candidateId, const std::vector<BYTE_T>& encrypted);
        bool verifyTallyDecryption(unsigned long int decrypted, const std::vector<BYTE_T>& encrypted, std::vector<BYTE_T>& r);
};
