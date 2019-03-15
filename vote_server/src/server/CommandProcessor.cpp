#include <iostream>

#include "shared_c/crypto/Cryptography.h"
#include "CommandProcessor.h"

std::pair<bool, std::vector<BYTE_T>> finishResponse(Response response, const Config& config) {
        std::vector<BYTE_T> vec; 

        // Copy over pubkey
        if(response.pubkey.size < config.pubKey().size()) {
                return {false, vec};
        }
        response.pubkey.size = config.pubKey().size();
        memcpy(response.pubkey.bytes, &(config.pubKey()[0]), response.pubkey.size);

        // Sign data
        int res = rsaSign(response.data.bytes, response.data.size, &(config.privKey()[0]), config.privKey().size(), response.signature.bytes, response.signature.size);
        if(res == CRYPTO_ERROR) {
                return {false, vec};
        } else {
                response.signature.size = res;
        }

        // Encode response
        size_t encodedSize = 0;
        bool resB = pb_get_encoded_size(&encodedSize, Response_fields, &response);
        if(!resB) {
                return {false, vec};
        }
        vec.resize(encodedSize + 2); // 2 bytes for size field
        pb_ostream_t buf = pb_ostream_from_buffer(&vec[0], encodedSize);
        resB = pb_encode_delimited(&buf, Response_fields, &response);
        if(!resB) {
                return {false, vec};
        }
        

        return {true, vec};
}

std::pair<bool, std::vector<BYTE_T>> errorResponse(const std::string& error, const Config& config) {
        // TODO
}


std::pair<bool, std::vector<BYTE_T>> processCommand(const std::vector<BYTE_T>& command, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        // Parse Command
        pb_istream_t pbBuf = pb_istream_from_buffer(&(command[0]), command.size());
        Command commandParsed;
        bool res = pb_decode_delimited(&pbBuf, Command_fields, &commandParsed);
        if(!res) {
                return errorResponse("Invalid Command!", config);
        }

        // Check if public key is in database
        pqxx::work txn(dbConn);
        pqxx::result r = txn.exec(
                "SELECT *"
                "FROM VOTERS"
                "WHERE SMARTCARD_PUBLIC_KEY = " + txn.esc_raw(commandParsed.pubkey.bytes, commandParsed.pubkey.size));
        if(r.size() != 1) {
                return errorResponse("Invalid Voter!", config);
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
                return errorResponse("Invalid signature!", config);
        }
        
        // TODO: Handle each command type
        std::pair<bool, std::vector<BYTE_T>> response;
        switch(commandParsed.type) {
                case(CommandType_GET_ELECTIONS):
                {
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
                        response = errorResponse("Invalid Command!", config);
                        break;
                }
        }
        return response;
}

Elections getElections();
Election getElection(const Election& election);
Voters getVoters();
Voter getVoter(const Voter& voter);
PlaintextBallots getPlaintextBallots();
PlaintextBallot getPlaintextBallot(const PlaintextBallot& plaintextBallot);
EncryptedBallots getEncryptedBallots();
PlaintextBallot getEncryptedBallot(const PlaintextBallot& plaintextBallot);
EncryptedBallot castBallot(const EncryptedBallot& tentativeBallot);
