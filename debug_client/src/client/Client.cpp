#include "Client.h"
#include "shared_c/crypto/Cryptography.h"
#include "shared_c/sockets/Sockets.h"
#include "shared_cpp/Encoding.h"

#include <iostream>
#include <cmath>

bool Client::sendCommand(int sock, CommandType commandType, const std::vector<BYTE_T>& data) const {
        // Construct Command
        Command command;
        command.type = commandType;

        if(data.size() < command.data.size) {
                return false;
        }
        command.data.size = data.size();
        memcpy(command.data.bytes, &(data[0]), data.size());

        // Copy over pubkey
        if(command.pubkey.size < _config->clientPubKey().size()) {
                return false;
        }
        command.pubkey.size = _config->clientPubKey().size();
        memcpy(command.pubkey.bytes, &(_config->clientPubKey()[0]), command.pubkey.size);

        // Sign type + data
        int commandTypeAndDataLen = sizeof(command.type) + command.data.size;
        unsigned char commandTypeAndData[commandTypeAndDataLen];
        memcpy(
                &commandTypeAndData, 
                &command.type, 
                sizeof(command.type));
        memcpy(
                &commandTypeAndData + sizeof(command.type), 
                command.data.bytes, 
                command.data.size);
        int res = rsaSign(commandTypeAndData, commandTypeAndDataLen, &(_config->clientPrivKey()[0]), _config->clientPrivKey().size(), command.signature.bytes, command.signature.size);
        if(res == CRYPTO_ERROR) {
                return false;
        } else {
                command.signature.size = res;
        }

        // Encode command
        std::pair<bool, std::vector<BYTE_T>> commandEnc = encodeMessage(Command_fields, command);
        if(!commandEnc.first) {
                return false;
        }

        // Send Command size and data over socket
        unsigned int commandSize = htonl(commandEnc.second.size());
        res = socketSend(sock, (unsigned char*) &commandSize, sizeof(unsigned int));
        if(!res) {
                return false;
        }
        res = socketSend(sock, &(commandEnc.second[0]), commandEnc.second.size());
        if(!res) {
                return false;
        }

        return true;
}

std::pair<bool, Response> Client::getResponse(int sock) const {
        // Receive Response length and data
        unsigned int msgLen;
        int res = socketRecv(sock, (BYTE_T*) &msgLen, sizeof(unsigned int));
        if(res < 0) {
                return {false, {}};
        }
        msgLen = ntohl(msgLen);
        std::vector<BYTE_T> msgBuf(msgLen);
        res = socketRecv(sock, &(msgBuf[0]), msgLen);
        if(res < 0) {
                return {false, {}};
        }

        // Parse response
        pb_istream_t pbBuf = pb_istream_from_buffer(&(msgBuf[0]), msgBuf.size());
        Response responseParsed;
        bool resB = pb_decode_delimited(&pbBuf, Response_fields, &responseParsed);
        if(!resB) {
                return {false, {}};
        }

        // Validate pubkey
        if(responseParsed.pubkey.size != _config->serverPubKey().size()) {
                return {false, {}};
        }
        if(memcmp(responseParsed.pubkey.bytes, &(_config->serverPubKey()[0]), responseParsed.pubkey.size) != 0) {
                return {false, {}};
        }

        // Validate signature
        int responseTypeAndDataLen = sizeof(responseParsed.type) + responseParsed.data.size;
        unsigned char responseTypeAndData[responseTypeAndDataLen];
        memcpy(
                &responseTypeAndData, 
                &responseParsed.type, 
                sizeof(responseParsed.type));
        memcpy(
                &responseTypeAndData + sizeof(responseParsed.type), 
                responseParsed.data.bytes, 
                responseParsed.data.size);
        bool validSig = rsaVerify(
                responseTypeAndData, 
                responseTypeAndDataLen, 
                responseParsed.signature.bytes, 
                responseParsed.signature.size, 
                responseParsed.pubkey.bytes, 
                responseParsed.pubkey.size);
        if(!validSig) {
                return {false, {}};
        }

        return {true, responseParsed};
}

void Client::start() {
        while(true) {
                _logger->info("Enter a command now.");
                std::string command;
                std::getline(std::cin, command);
                if(command.find("create_keypair") == 0) {
                        // Create key
                        BYTE_T pubKey[RSA_KEY_SIZE_DER];
                        unsigned int pubKeyLen = RSA_KEY_SIZE_DER;
                        BYTE_T privKey[RSA_KEY_SIZE_DER];
                        unsigned int privKeyLen = RSA_KEY_SIZE_DER;
                        int res = createKeypair(RSA_KEY_SIZE, pubKey, &pubKeyLen, privKey, &privKeyLen);
                        if(res != 0) {
                                _logger->error("Crypto error 1!");
                                continue;
                        }

                        // Alloc mem for key encode
                        // TODO: these lens are too big 
                        unsigned int pubKeyEncodeLen = 2 * std::ceil((pubKeyLen / (double) 3)) * 4;
                        BYTE_T pubKeyEncode[pubKeyEncodeLen];
                        unsigned int privKeyEncodeLen = 2 * std::ceil((privKeyLen / (double) 3)) * 4;
                        BYTE_T privKeyEncode[privKeyEncodeLen];

                        // Encode key
                        res = Base64_Encode(
                                pubKey,
                                pubKeyLen,
                                pubKeyEncode,
                                &pubKeyEncodeLen
                        );
                        if(res != 0) {
                                _logger->error("Crypto error 2!");
                                continue;
                        }
                        res = Base64_Encode(
                                privKey,
                                privKeyLen,
                                privKeyEncode,
                                &privKeyEncodeLen
                        );
                        if(res != 0) {
                                _logger->error("Crypto error 3!");
                                continue;
                        }
                
                
                        _logger->info("Public key: " + std::string((char*) pubKeyEncode, pubKeyEncodeLen));
                        _logger->info("Private key: " + std::string((char*) privKeyEncode, privKeyEncodeLen));
                } else { 
                        // Create new socket
                        int mainSock = socket(AF_INET, SOCK_STREAM, 0);
        
                        // Create server addr
                        sockaddr_in serv_addr;
                        bzero((char *) &serv_addr, sizeof(serv_addr));
                        serv_addr.sin_family = AF_INET;
                        serv_addr.sin_port = htons(_config->serverPort());
                        int res = inet_pton(AF_INET, _config->serverHost().c_str(), &serv_addr.sin_addr);
                        if(res <= 0) {
                                _logger->error("Failed to start client - invalid host!");
                                continue;
                        }
        
                        // Connect
                        res = connect(mainSock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
                        if(res < 0) {
                                _logger->error("Failed to start client - unable to connect!");
                                continue;
                        }

                        if(command.find("get_elections") == 0) {
                                getElections(mainSock);
                        } else {
                                _logger->error("Invalid command!");
                        }
        
                        // Close socket and restart
                        close(mainSock);
                }
        }
}

void Client::getElections(int sock) const {
        // Construct PaginationMetadata and add to Command
        PaginationMetadata pagination;
        pagination.lastId = 0;
        std::pair<bool, std::vector<BYTE_T>> paginationEnc = encodeMessage<PaginationMetadata>(PaginationMetadata_fields, pagination);
        if(!paginationEnc.first) {
                _logger->error("Unable to generate PaginationMetadata!");
                return;
        }

        // Sign Command and send to server
        bool res = sendCommand(sock, CommandType_GET_ELECTIONS, paginationEnc.second);
        if(!res) {
                _logger->error("Unable to send Command!");
                return;
        }

        // Retrieve Response
        std::pair<bool, Response> resp = getResponse(sock);
        if(!resp.first) {
                _logger->error("Unable to retrieve Response!");
                return;
        }

        // Validate Response
        if(resp.second.type != ResponseType_ELECTIONS) {
                _logger->error("Invalid Response type!");
                return;
        }

        // Parse out Elections
        Elections electionsParsed;
        pb_istream_t pbBuf = pb_istream_from_buffer(&(resp.second.data.bytes[0]), resp.second.data.size);
        bool resB = pb_decode_delimited(&pbBuf, Elections_fields, &electionsParsed);
        if(!resB) {
                _logger->error("Unable to parse Elections!");
                return;
        }

        // Output Elections
        for(int e = 0; e < electionsParsed.elections_count; e++) {
                Election* election = &(electionsParsed.elections[e]);
                _logger->info("Election " + std::to_string(e) + ":");
                _logger->info("    ID: " + std::to_string(election->id));
                _logger->info("    Start time UTC: " + std::to_string(election->start_time_utc));
                _logger->info("    End time UTC: " + std::to_string(election->end_time_utc));
                _logger->info("    Enabled: " + std::to_string(election->enabled));
                _logger->info("    Allow write in: " + std::to_string(election->allow_write_in));
                for(int avg = 0; avg < election->authorized_voter_group_ids_count; avg++) {
                        _logger->info("    Authorized voter group " + std::to_string(avg) + ": " + std::to_string(election->authorized_voter_group_ids[avg]));
                }
                for(int cand = 0; cand < election->candidates_count; cand++) {
                        Candidate* c = &(election->candidates[cand]);
                        _logger->info("    Candidate " + std::to_string(cand) + " ID: " + std::to_string(c->id));
                        _logger->info("    Candidate " + std::to_string(cand) + " first name: " + c->first_name);
                        _logger->info("    Candidate " + std::to_string(cand) + " last name: " + c->last_name);
                }
        }
}
