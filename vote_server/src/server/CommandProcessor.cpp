#include <iostream>

#include "shared_c/crypto/Cryptography.h"
#include "shared_cpp/Encoding.h"
#include "CommandProcessor.h"

std::pair<bool, std::vector<BYTE_T>> finishResponse(Response response, Logger& logger, const Config& config) {
        // Copy over pubkey
        if(sizeof(response.pubkey) < config.pubKey().size()) {
                logger.error("finishResponse error 1!");
                return {false, {}};
        }
        response.pubkey.size = config.pubKey().size();
        memcpy(response.pubkey.bytes, &(config.pubKey()[0]), response.pubkey.size);

        // Sign type + data
        int responseTypeAndDataLen = sizeof(response.type) + response.data.size;
        unsigned char responseTypeAndData[responseTypeAndDataLen];
        memcpy(
                &responseTypeAndData, 
                &response.type, 
                sizeof(response.type));
        memcpy(
                &responseTypeAndData + sizeof(response.type), 
                response.data.bytes, 
                response.data.size);
        int res = rsaSign(responseTypeAndData, responseTypeAndDataLen, &(config.privKey()[0]), config.privKey().size(), response.signature.bytes, response.signature.size);
        if(res == CRYPTO_ERROR) {
                logger.error("finishResponse error 2!");
                return {false, {}};
        } else {
                response.signature.size = res;
        }

        return encodeMessage(Response_fields, response);
}

std::pair<bool, std::vector<BYTE_T>> errorResponse(const std::string& error, Logger& logger, const Config& config) {
        Response resp;
        if(resp.data.size < error.length() + 1) {
                return {false, {}};
        }
        resp.type = ResponseType_ERROR;
        resp.data.size = error.length() + 1;
        memcpy(resp.data.bytes, error.c_str(), resp.data.size);

        return finishResponse(resp, logger, config);
}

std::pair<bool, std::vector<BYTE_T>> getElections(const PaginationMetadata& pagination, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        pqxx::work txn(dbConn);
        pqxx::result r = txn.exec(
                "SELECT e.id, e.start_time, e.end_time, e.enabled, e.allow_write_in"
                " FROM elections e"
                " WHERE e.id > " + std::to_string(pagination.lastId) +
                " ORDER BY e.id ASC");

        Elections elections;
        elections.elections_count = r.size();
        for(int i = 0; i < r.size(); i++) {
                elections.elections[i].id = r[i][0].as<int>();
                elections.elections[i].start_time_utc = r[i][1].as<int>();
                elections.elections[i].end_time_utc = r[i][2].as<int>();
                elections.elections[i].enabled = r[i][3].as<bool>();
                elections.elections[i].allow_write_in = r[i][4].as<bool>();

                pqxx::result evg_r = txn.exec(
                        "SELECT evg.id"
                        " FROM elections_voter_groups evg"
                        " WHERE evg.id = " + std::to_string(elections.elections[i].id) +
                        " ORDER BY evg.id ASC");

                elections.elections[i].authorized_voter_group_ids_count = evg_r.size();
                for(int j = 0; j < evg_r.size(); j++) {
                        elections.elections[i].authorized_voter_group_ids[j] = evg_r[j][0].as<int>();
                }

                pqxx::result cand_r = txn.exec(
                        "SELECT c.id, c.first_name, c.last_name"
                        " FROM candidates c"
                        " WHERE c.id = " + std::to_string(elections.elections[i].id) +
                        "ORDER BY c.id ASC");

                elections.elections[i].candidates_count = cand_r.size();
                for(int j = 0; j < cand_r.size(); j++) {
                        int returned_id = cand_r[j][0].as<int>();
                        std::string returned_fname = cand_r[j][1].as<std::string>();
                        std::string returned_lname = cand_r[j][2].as<std::string>();

                        Candidate* cand = &(elections.elections[i].candidates[j]);
                        cand->id = returned_id;

                        if(cand->first_name.size < returned_fname.length() + 1) {
                                return {false, {}};
                        }
                        cand->first_name.size = returned_fname.length() + 1;
                        memcpy(cand->first_name.bytes, returned_fname.c_str(), cand->first_name.size);

                        if(cand->last_name.size < returned_lname.length() + 1) {
                                return {false, {}};
                        }
                        cand->last_name.size = returned_lname.length() + 1;
                        memcpy(cand->last_name.bytes, returned_lname.c_str(), cand->last_name.size);
                }
        }

        // Encode and return response
        Response resp;
        resp.type = ResponseType_ELECTIONS;
        std::pair<bool, std::vector<BYTE_T>> electionsEncoded = encodeMessage<Elections>(Elections_fields, elections);
        if(!electionsEncoded.first || electionsEncoded.second.size() > resp.data.size) {
                logger.error("Unable to encode elections!");
                return {false, {}};
        }
        resp.data.size = electionsEncoded.second.size();
        memcpy(resp.data.bytes, &(electionsEncoded.second[0]), electionsEncoded.second.size());

        logger.info("Get elections complete!");
        return finishResponse(resp, logger, config);
}

std::pair<bool, std::vector<BYTE_T>> processCommand(const std::vector<BYTE_T>& command, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        logger.info("Processing command");

        // Parse Command
        pb_istream_t pbBuf = pb_istream_from_buffer(&(command[0]), command.size());
        Command commandParsed;
        bool res = pb_decode_delimited(&pbBuf, Command_fields, &commandParsed);
        if(!res) {
                logger.info("Invalid command!");
                return errorResponse("Invalid Command!", logger, config);
        }
        pb_istream_t dataBuf = pb_istream_from_buffer(commandParsed.data.bytes, commandParsed.data.size);

        // Check if public key is in database
        {
                pqxx::work txn(dbConn);
                pqxx::result r = txn.exec(
                        "SELECT *"
                        " FROM VOTERS"
                        " WHERE SMARTCARD_PUBLIC_KEY = " + txn.quote(txn.esc_raw(commandParsed.pubkey.bytes, commandParsed.pubkey.size)));
                if(r.size() != 1) {
                        logger.info("Invalid voter!");
                        return errorResponse("Invalid Voter!", logger, config);
                }
        }

        // Validate signature
        int commandTypeAndDataLen = sizeof(commandParsed.type) + commandParsed.data.size;
        unsigned char commandTypeAndData[commandTypeAndDataLen];
        memcpy(
                &commandTypeAndData, 
                &commandParsed.type, 
                sizeof(commandParsed.type));
        memcpy(
                &commandTypeAndData + sizeof(commandParsed.type), 
                commandParsed.data.bytes, 
                commandParsed.data.size);
        bool validSig = rsaVerify(
                commandTypeAndData, 
                commandTypeAndDataLen, 
                commandParsed.signature.bytes, 
                commandParsed.signature.size, 
                commandParsed.pubkey.bytes, 
                commandParsed.pubkey.size);
        if(!validSig) {
                logger.info("Invalid signature!");
                return errorResponse("Invalid signature!", logger, config);
        }
        
        // TODO: Handle each command type
        switch(commandParsed.type) {
                case(CommandType_GET_ELECTIONS):
                {
                        PaginationMetadata pagination;
                        bool res = pb_decode_delimited(&dataBuf, PaginationMetadata_fields, &pagination);

                        if(!res) {
                                logger.info("Invalid pagination data!");
                                return errorResponse("Invalid pagination data!", logger, config);
                        }

                        return getElections(pagination, dbConn, logger, config);
                }
                case(CommandType_GET_VALID_VOTER_GROUPS):
                {
                }
                case(CommandType_GET_VALID_VOTERS):
                {
                }
                case(CommandType_GET_PLAINTEXT_BALLOTS):
                {
                }
                case(CommandType_GET_ENCRYPTED_BALLOTS):
                {
                }
                case(CommandType_GET_PLAINTEXT_BALLOT):
                {
                }
                case(CommandType_GET_ENCRYPTED_BALLOT):
                {
                }
                case(CommandType_CAST_BALLOT):
                {
                }
                default:
                {
                        logger.info("Invalid Command!");
                        return errorResponse("Invalid Command!", logger, config);
                }
        }
}
