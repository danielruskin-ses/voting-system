#include "Client.h"
#include "shared_c/crypto/Cryptography.h"
#include "shared_c/sockets/Sockets.h"
#include "shared_cpp/Encoding.h"

#include <iostream>
#include <cmath>

#define SOCKET_TIMEOUT 5

int Client::newConn() {
        // Create new socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        
        // Create server addr
        sockaddr_in serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(_config->serverPort());
        int res = inet_pton(AF_INET, _config->serverHost().c_str(), &serv_addr.sin_addr);
        if(res <= 0) {
                _logger->error("Failed to start client - invalid host!");
                return -1;
        }
        
        // Connect
        res = connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        if(res < 0) {
                _logger->error("Failed to start client - unable to connect!");
                return -1;
        }

        return sock;
}

bool Client::sendCommand(int sock, CommandType commandType, std::vector<BYTE_T>& data) {
        // Construct Command
        Command command;
        command.type = commandType;
        command.data.arg = &data;
        command.data.funcs.encode = ByteTArrayEncodeFunc;

        // Copy over pubkey
        command.pubkey.arg = &_clientPubKey;
        command.pubkey.funcs.encode = ByteTArrayEncodeFunc;

        // Sign type + data
        int commandTypeAndDataLen = sizeof(command.type) + data.size();
        unsigned char commandTypeAndData[commandTypeAndDataLen];
        memcpy(
                &commandTypeAndData, 
                &command.type, 
                sizeof(command.type));
        memcpy(
                commandTypeAndData + sizeof(command.type), 
                &(data[0]), 
                data.size());
        std::vector<BYTE_T> signature(RSA_SIGNATURE_SIZE);
        int res = rsaSign(commandTypeAndData, commandTypeAndDataLen, &(_config->clientPrivKey()[0]), _config->clientPrivKey().size(), &(signature[0]), signature.size());
        if(res == CRYPTO_ERROR) {
                return false;
        } else {
                signature.resize(res);
                command.signature.arg = (void*) (&signature);
                command.signature.funcs.encode = ByteTArrayEncodeFunc;
        }

        // Encode command
        std::pair<bool, std::vector<BYTE_T>> commandEnc = encodeMessage(Command_fields, command);
        if(!commandEnc.first) {
                return false;
        }

        // Send Command size and data over socket
        unsigned int commandSize = htonl(commandEnc.second.size());
        res = socketSend(sock, (unsigned char*) &commandSize, sizeof(unsigned int), SOCKET_TIMEOUT);
        if(res != 0) {
                return false;
        }
        res = socketSend(sock, &(commandEnc.second[0]), commandEnc.second.size(), SOCKET_TIMEOUT);
        if(res != 0) {
                return false;
        }

        return true;
}

std::tuple<bool, ResponseType, std::vector<BYTE_T>> Client::getResponse(int sock) {
        // Receive Response length and data
        unsigned int msgLen;
        int res = socketRecv(sock, (BYTE_T*) &msgLen, sizeof(unsigned int), SOCKET_TIMEOUT);
        if(res < 0) {
                _logger->error("getResponse error 1!");
                return {false, ResponseType_ERROR, {}};
        }
        msgLen = ntohl(msgLen);
        std::vector<BYTE_T> msgBuf(msgLen);
        res = socketRecv(sock, &(msgBuf[0]), msgLen, SOCKET_TIMEOUT);
        if(res < 0) {
                _logger->error("getResponse error 2!");
                return {false, ResponseType_ERROR, {}};
        }

        // Prepare response for parsing
        std::vector<BYTE_T> responseData;
        std::vector<BYTE_T> responsePubKey;
        std::vector<BYTE_T> responseSignature;
        Response responseParsed;
        responseParsed.data.arg = &responseData;
        responseParsed.data.funcs.decode = ByteTArrayDecodeFunc;
        responseParsed.pubkey.arg = &responsePubKey;
        responseParsed.pubkey.funcs.decode = ByteTArrayDecodeFunc;
        responseParsed.signature.arg = &responseSignature;
        responseParsed.signature.funcs.decode = ByteTArrayDecodeFunc;
        
        // Parse response
        pb_istream_t pbBuf = pb_istream_from_buffer(&(msgBuf[0]), msgBuf.size());
        bool resB = pb_decode_delimited(&pbBuf, Response_fields, &responseParsed);
        if(!resB) {
                _logger->error("getResponse error 3!");
                return {false, ResponseType_ERROR, {}};
        }

        // Validate pubkey
        if(responsePubKey != _config->serverPubKey()) {
                _logger->error("getResponse error 4!");
                return {false, ResponseType_ERROR, {}};
        }

        // Validate signature
        int responseTypeAndDataLen = sizeof(responseParsed.type) + responseData.size();
        unsigned char responseTypeAndData[responseTypeAndDataLen];
        memcpy(
                &responseTypeAndData, 
                &responseParsed.type, 
                sizeof(responseParsed.type));
        memcpy(
                responseTypeAndData + sizeof(responseParsed.type), 
                &(responseData[0]), 
                responseData.size());
        bool validSig = rsaVerify(
                responseTypeAndData, 
                responseTypeAndDataLen, 
                &(responseSignature[0]),
                responseSignature.size(),
                &(responsePubKey[0]),
                responsePubKey.size());
        if(!validSig) {
                _logger->error("getResponse error 6!");
                return {false, ResponseType_ERROR, {}};
        }

        return {true, responseParsed.type, responseData};
}

std::pair<std::string, std::string> Client::createPaillierKeypair() {
        char* privHex;
        char* pubHex;

        paillierKeygen(P_KEY_SIZE, &privHex, &pubHex);
        std::pair<std::string, std::string> res = {std::string(pubHex), std::string(privHex)};

        free(privHex);
        free(pubHex);

        return res;
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
                } else if(command.find("create_paillier_keypair") == 0) {
                        std::pair<std::string, std::string> kp = createPaillierKeypair();

                        _logger->info("Public key: " + kp.first);
                        _logger->info("Private key: " + kp.second);
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
                } else if(command.find("get_elections") == 0) {
                        getElections(true);
                } else if(command.find("cast_ballot") == 0) {
                        castBallot();
                } else {
                        _logger->error("Invalid command!");
                }
        }
}

void Client::castBallot() {
        // Enter election ID
        _logger->info("Enter an election ID.");
        std::string eidS;
        std::getline(std::cin, eidS);
        int eid = std::stoi(eidS);

        // Fetch elections
        std::tuple<bool, std::vector<Election>, std::vector<std::vector<int>>, std::vector<std::vector<std::tuple<Candidate, std::string, std::string>>>> elections = getElections(false);
        if(!std::get<0>(elections)) {
                _logger->error("Unable to fetch elections!");
                return;
        }

        // Find election with correct ID
        int electionIdx = -1;
        for(int i = 0; i < std::get<1>(elections).size(); i++) {
                if(std::get<1>(elections)[i].id == eid) {
                        electionIdx = i;
                        break;
                }
        }
        if(electionIdx == -1) {
                _logger->error("Invalid election ID!");
                return;
        }

        // Create ballot and populate candidate choices
        std::vector<EncryptedBallotEntry> encryptedBallotEntries(std::get<3>(elections)[electionIdx].size());
        std::vector<std::vector<BYTE_T>> ciphertexts(encryptedBallotEntries.size());
        for(int i = 0; i < encryptedBallotEntries.size(); i++) {
                // Fetch user choice
                _logger->info("Enter choice for candidate " + std::to_string(std::get<0>(std::get<3>(elections)[electionIdx][i]).id) + ":");
                std::string choiceS;
                std::getline(std::cin, choiceS);
                int choice = std::stoi(choiceS);

                // Encrypt user choice
                void* ctext = NULL;
                ciphertexts[i].resize(P_CIPHERTEXT_MAX_LEN);
                paillierEnc(choice, &(_config->serverPaillierPubKey()[0]), &ctext);
                memcpy(&(ciphertexts[i][0]), ctext, P_CIPHERTEXT_MAX_LEN);

                // Add to ballot
                encryptedBallotEntries[i].encrypted_value.arg = &(ciphertexts[i]);
                encryptedBallotEntries[i].encrypted_value.funcs.encode = ByteTArrayEncodeFunc;
                encryptedBallotEntries[i].candidate_id = std::get<0>(std::get<3>(elections)[electionIdx][i]).id;

                // Free mem
                free(ctext);
        }

        // Prepare EncryptedBallot for encoding
        EncryptedBallot eb;
        eb.election_id = eid;
        eb.encrypted_ballot_entries.arg = &encryptedBallotEntries;
        eb.encrypted_ballot_entries.funcs.encode = RepeatedEncryptedBallotEntryEncodeFunc;

        // Encode msg and send to server
        int sock = newConn();
        if(sock == -1) {
                return;
        }

        std::pair<bool, std::vector<BYTE_T>> ballotEnc = encodeMessage<EncryptedBallot>(EncryptedBallot_fields, eb);
        if(!ballotEnc.first) {
                _logger->error("Unable to encode EncryptedBallot!");
                return;
        }
        bool res = sendCommand(sock, CommandType_CAST_BALLOT, ballotEnc.second);
        if(!res) {
                _logger->error("Unable to send Command!");
                return;
        }

        // Retrieve Response
        std::tuple<bool, ResponseType, std::vector<BYTE_T>> resp = getResponse(sock);
        if(!std::get<0>(resp)) {
                _logger->error("Unable to retrieve Response!");
                return;
        }
        close(sock);

        // Validate Response
        if(std::get<1>(resp) != ResponseType_CAST_ENCRYPTED_BALLOT) {
                _logger->error("Invalid Response type!");
                return;
        }

        // Parse out CastEncryptedBallot
        CastEncryptedBallot ceb;
        ceb.encrypted_ballot.encrypted_ballot_entries.funcs.decode = NULL;
        ceb.cast_command_data.funcs.decode = NULL;
        ceb.voter_signature.funcs.decode = NULL;
        pb_istream_t pbBuf = pb_istream_from_buffer(&(std::get<2>(resp)[0]), std::get<2>(resp).size());
        bool resB = pb_decode_delimited(&pbBuf, CastEncryptedBallot_fields, &ceb);
        if(!resB) {
                _logger->error("Unable to parse CastEncryptedBallot!");
                return;
        }

        // Output CastEncryptedBallot
        _logger->info("CastEncryptedBallot " + std::to_string(ceb.id) + ":");
        _logger->info("    Voter ID: " + std::to_string(ceb.voter_id));
        _logger->info("    Cast at: " + std::to_string(ceb.cast_at));
}

std::tuple<bool, std::vector<Election>, std::vector<std::vector<int>>, std::vector<std::vector<std::tuple<Candidate, std::string, std::string>>>> Client::getElections(bool output) {
        int sock = newConn();
        if(sock == -1) {
                return {false, {}, {}, {}};
        }

        // Construct PaginationMetadata and add to Command
        PaginationMetadata pagination;
        pagination.lastId = 0;
        std::pair<bool, std::vector<BYTE_T>> paginationEnc = encodeMessage<PaginationMetadata>(PaginationMetadata_fields, pagination);
        if(!paginationEnc.first) {
                _logger->error("Unable to generate PaginationMetadata!");
                return {false, {}, {}, {}};
        }

        // Sign Command and send to server
        bool res = sendCommand(sock, CommandType_GET_ELECTIONS, paginationEnc.second);
        if(!res) {
                _logger->error("Unable to send Command!");
                return {false, {}, {}, {}};
        }

        // Retrieve Response
        std::tuple<bool, ResponseType, std::vector<BYTE_T>> resp = getResponse(sock);
        if(!std::get<0>(resp)) {
                _logger->error("Unable to retrieve Response!");
                return {false, {}, {}, {}};
        }

        // Validate Response
        if(std::get<1>(resp) != ResponseType_ELECTIONS) {
                _logger->error("Invalid Response type!");
                return {false, {}, {}, {}};
        }

        // Prepare Elections for decoding
        Elections elections;
        std::vector<Election> electionsArr;
        std::vector<std::vector<int>> authVoterGroupArr;
        std::vector<std::vector<std::tuple<Candidate, std::string, std::string>>> candidatesArr;
        std::tuple<std::vector<Election>*, std::vector<std::vector<int>>*, std::vector<std::vector<std::tuple<Candidate, std::string, std::string>>>*> electionsDecodeArgs = { &electionsArr, &authVoterGroupArr, &candidatesArr };
        elections.elections.arg = &electionsDecodeArgs;
        elections.elections.funcs.decode = ElectionsDecodeFunc;

        // Decode Elections
        pb_istream_t pbBuf = pb_istream_from_buffer(&(std::get<2>(resp)[0]), std::get<2>(resp).size());
        bool resB = pb_decode_delimited(&pbBuf, Elections_fields, &elections);
        if(!resB) {
                _logger->error("Unable to parse Elections!");
                return {false, {}, {}, {}};
        }

        // Output Elections
        if(output) {
                for(int e = 0; e < electionsArr.size(); e++) {
                        _logger->info("Election " + std::to_string(e) + ":");
                        _logger->info("    ID: " + std::to_string(electionsArr[e].id));
                        _logger->info("    Start time UTC: " + std::to_string(electionsArr[e].start_time_utc));
                        _logger->info("    End time UTC: " + std::to_string(electionsArr[e].end_time_utc));
                        _logger->info("    Enabled: " + std::to_string(electionsArr[e].enabled));
                        _logger->info("    Allow write in: " + std::to_string(electionsArr[e].allow_write_in));
                        for(int avg = 0; avg < authVoterGroupArr[e].size(); avg++) {
                                _logger->info("    Authorized voter group " + std::to_string(avg) + ": " + std::to_string(authVoterGroupArr[e][avg]));
                        }
                        for(int cand = 0; cand < candidatesArr[e].size(); cand++) {
                                _logger->info("    Candidate " + std::to_string(cand) + " ID: " + std::to_string(std::get<0>(candidatesArr[e][cand]).id));
                                _logger->info("    Candidate " + std::to_string(cand) + " first name: " + std::get<1>(candidatesArr[e][cand]));
                                _logger->info("    Candidate " + std::to_string(cand) + " last name: " + std::get<2>(candidatesArr[e][cand]));
                        }
                }
        }

        close(sock);

        return {true, electionsArr, authVoterGroupArr, candidatesArr};
}
