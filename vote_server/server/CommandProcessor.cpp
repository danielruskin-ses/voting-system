#include <iostream>

#include "CommandProcessor.h"

std::vector<uint8_t> finishResponse(const Response& response) {
        // TODO
}

Response errorResponse(const std::string& error) {
        // TODO
}


std::vector<uint8_t> processCommand(const std::vector<uint8_t>& command) {
        // Parse Command
        pb_istream_t pbBuf = pb_istream_from_buffer(&(command[0]), command.size());
        Command commandParsed;
        bool res = pb_decode_delimited(&pbBuf, Command_fields, &commandParsed);
        if(!res) {
                return finishResponse(errorResponse("Invalid Command!"));
        }

        // TODO: check authorization
        
        // Handle each command type
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
