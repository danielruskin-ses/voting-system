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

        if(sizeof(command.data.bytes) < data.size()) {
                return false;
        }
        command.data.size = data.size();
        memcpy(command.data.bytes, &(data[0]), data.size());

        // Copy over pubkey
        if(sizeof(command.pubkey.bytes) < _config->clientPubKey().size()) {
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
        int res = rsaSign(commandTypeAndData, commandTypeAndDataLen, &(_config->clientPrivKey()[0]), _config->clientPrivKey().size(), command.signature.bytes, sizeof(command.signature.bytes));
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
        if(res != 0) {
                return false;
        }
        res = socketSend(sock, &(commandEnc.second[0]), commandEnc.second.size());
        if(res != 0) {
                return false;
        }

        return true;
}

std::pair<bool, Response> Client::getResponse(int sock) const {
        // Receive Response length and data
        unsigned int msgLen;
        int res = socketRecv(sock, (BYTE_T*) &msgLen, sizeof(unsigned int));
        if(res < 0) {
                _logger->error("getResponse error 1!");
                return {false, {}};
        }
        msgLen = ntohl(msgLen);
        std::vector<BYTE_T> msgBuf(msgLen);
        res = socketRecv(sock, &(msgBuf[0]), msgLen);
        if(res < 0) {
                _logger->error("getResponse error 2!");
                return {false, {}};
        }

        // Parse response
        pb_istream_t pbBuf = pb_istream_from_buffer(&(msgBuf[0]), msgBuf.size());
        Response responseParsed;
        bool resB = pb_decode_delimited(&pbBuf, Response_fields, &responseParsed);
        if(!resB) {
                _logger->error("getResponse error 3!");
                return {false, {}};
        }

        // Validate pubkey
        if(responseParsed.pubkey.size != _config->serverPubKey().size()) {
                _logger->error("getResponse error 4!");
                return {false, {}};
        }
        if(memcmp(responseParsed.pubkey.bytes, &(_config->serverPubKey()[0]), responseParsed.pubkey.size) != 0) {
                _logger->error("getResponse error 5!");
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
                responseTypeAndData + sizeof(responseParsed.type), 
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
                _logger->error("getResponse error 6!");
                return {false, {}};
        }

        return {true, responseParsed};
}

std::tuple<bool, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>> Client::createKeypair() {
        // Create key
        std::vector<BYTE_T> pubKey(RSA_PUBLIC_KEY_SIZE_DER);
        unsigned int pubKeyLen = RSA_PUBLIC_KEY_SIZE_DER;
        std::vector<BYTE_T> privKey(RSA_PRIVATE_KEY_SIZE_DER);
        unsigned int privKeyLen = RSA_PRIVATE_KEY_SIZE_DER;
        int res = generateKeypair(RSA_PRIVATE_KEY_SIZE, &(pubKey[0]), &pubKeyLen, &(privKey[0]), &privKeyLen);
        if(res != 0) {
                return {false, {}, {}, {}, {}};
        }
        pubKey.resize(pubKeyLen);
        privKey.resize(privKeyLen);
        
        // Alloc mem for key encode
        unsigned int pubKeyEncodeLen = RSA_PUBLIC_KEY_SIZE_B64;
        std::vector<BYTE_T> pubKeyEncode(pubKeyEncodeLen);
        unsigned int privKeyEncodeLen = RSA_PRIVATE_KEY_SIZE_B64;
        std::vector<BYTE_T> privKeyEncode(privKeyEncodeLen);
        
        // Encode key
        res = Base64_Encode(
                &(pubKey[0]),
                pubKeyLen,
                &(pubKeyEncode[0]),
                &pubKeyEncodeLen
        );
        if(res != 0) {
                return {false, {}, {}, {}, {}};
        }
        res = Base64_Encode(
                &(privKey[0]),
                privKeyLen,
                &(privKeyEncode[0]),
                &privKeyEncodeLen
        );
        if(res != 0) {
                return {false, {}, {}, {}, {}};
        }
        pubKeyEncode.resize(pubKeyEncodeLen);
        privKeyEncode.resize(privKeyEncodeLen);

        return {true, pubKey, pubKeyEncode, privKey, privKeyEncode};
}

void Client::start() {
        while(true) {
                _logger->info("Enter a command now.");
                std::string command;
                std::getline(std::cin, command);
                if(command.find("create_keypair") == 0) {
                        std::tuple<bool, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>> kp = createKeypair();

                        if(!std::get<0>(kp)) {
                                _logger->error("Error generating key!");
                                continue;
                        }
                
                        _logger->info("Public key: " + std::string((char*) &(std::get<2>(kp)[0]), std::get<2>(kp).size()));
                        _logger->info("Private key: " + std::string((char*) &(std::get<4>(kp)[0]), std::get<4>(kp).size()));
                } else if(command.find("create_voter") == 0) {
                        std::tuple<bool, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>, std::vector<BYTE_T>> kp = createKeypair();
                        if(!std::get<0>(kp)) {
                                _logger->error("Error generating key!");
                                continue;
                        }

                        auto dbConn = _database.getConnection();
                        pqxx::work txn(*dbConn);

                        // Get or create voter group with id 1
                        pqxx::result r = txn.exec("INSERT INTO VOTER_GROUPS (ID, NAME) VALUES (1, 'VOTER GROUP 1') ON CONFLICT (ID) DO NOTHING");
        
                        // Create voter
                        char zero[5] = {0,0,0,0,0};
                        r = txn.exec(
                                "INSERT INTO VOTERS (VOTER_GROUP_ID, FIRST_NAME, LAST_NAME, SMARTCARD_PUBLIC_KEY, REG_MATERIAL_HASH, REG_MATERIAL_IMG)"
                                "VALUES (1, 'Daniel', 'Ruskin', "
                                + txn.quote(txn.esc_raw(&(std::get<1>(kp)[0]), std::get<1>(kp).size())) + ", "
                                + txn.quote(txn.esc_raw((BYTE_T*) zero, sizeof(zero))) + ", "
                                + txn.quote(txn.esc_raw((BYTE_T*) zero, sizeof(zero))) + ");");

                        txn.commit();

                        _logger->info("Public key: " + std::string((char*) &(std::get<2>(kp)[0]), std::get<2>(kp).size()));
                        _logger->info("Private key: " + std::string((char*) &(std::get<4>(kp)[0]), std::get<4>(kp).size()));
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
                        _logger->info("    Candidate " + std::to_string(cand) + " first name: " + std::string((const char*) c->first_name.bytes, c->first_name.size));
                        _logger->info("    Candidate " + std::to_string(cand) + " last name: " + std::string((const char*) c->last_name.bytes, c->last_name.size));
                }
        }
}
