#include <iostream>

#include "shared_c/crypto/Cryptography.h"
#include "CommandProcessor.h"

std::vector<BYTE_T> finishResponse(const Response& response) {
        // TODO
}

Response errorResponse(const std::string& error) {
        // TODO
}


std::vector<BYTE_T> processCommand(const std::vector<BYTE_T>& command, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        // Parse Command
        pb_istream_t pbBuf = pb_istream_from_buffer(&(command[0]), command.size());
        Command commandParsed;
        bool res = pb_decode_delimited(&pbBuf, Command_fields, &commandParsed);
        if(!res) {
                return finishResponse(errorResponse("Invalid Command!"));
        }

        // Check if public key is in database
        pqxx::work txn(dbConn);
        pqxx::result r = txn.exec(
                "SELECT *"
                "FROM VOTERS"
                "WHERE SMARTCARD_PUBLIC_KEY = " + txn.esc_raw(commandParsed.pubkey.bytes, commandParsed.pubkey.size));
        if(r.size() != 1) {
                return finishResponse(errorResponse("Invalid Voter!"));
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
                return finishResponse(errorResponse("Invalid signature!"));
        }
        
        // TODO: Handle each command type
        Response response;
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
                        response = errorResponse("Invalid Command!");
                        break;
                }
        }
        return finishResponse(response);
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
