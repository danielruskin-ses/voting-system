#include <iostream>

#include "shared_c/crypto/Cryptography.h"
#include "shared_c/Time.h"
#include "shared_cpp/Encoding.h"
#include "CommandProcessor.h"

#include <iostream>

#define PAGINATION_MAX 100

std::pair<bool, std::vector<BYTE_T>> finishResponse(Response response, Logger& logger, const Config& config) {
        // Copy over pubkey
        response.pubkey.arg = (void*) &config.pubKey();
        response.pubkey.funcs.encode = ByteTArrayEncodeFunc;

        // Sign type + data
        const std::vector<BYTE_T>& dataArg = *((std::vector<BYTE_T>*) response.data.arg);
        int responseTypeAndDataLen = sizeof(response.type) + dataArg.size();
        unsigned char responseTypeAndData[responseTypeAndDataLen];
        std::vector<BYTE_T> signature(RSA_SIGNATURE_SIZE);
        memcpy(
                &responseTypeAndData, 
                &response.type, 
                sizeof(response.type));
        memcpy(
                responseTypeAndData + sizeof(response.type),
                &dataArg[0],
                dataArg.size());
        int res = rsaSign(responseTypeAndData, responseTypeAndDataLen, &(config.privKey()[0]), config.privKey().size(), &(signature[0]), signature.size());
        if(res == CRYPTO_ERROR) {
                logger.error("finishResponse error 2!");
                return {false, {}};
        } 
        signature.resize(res);
        response.signature.arg = (void*) &signature;
        response.signature.funcs.encode = ByteTArrayEncodeFunc; 

        return encodeMessage(Response_fields, response);
}

std::pair<bool, std::vector<BYTE_T>> errorResponse(const std::string& error, Logger& logger, const Config& config) {
        Response resp;
        resp.type = ResponseType_ERROR;
        
        std::vector<BYTE_T> vec(error.size() + 1);
        memcpy(&(vec[0]), error.c_str(), error.size() + 1);

        resp.data.arg = (void*) &vec;
        resp.data.funcs.encode = ByteTArrayEncodeFunc; 

        return finishResponse(resp, logger, config);
}

// TODO: handle write in, alt src ballots
std::pair<bool, std::vector<BYTE_T>> castBallot(const std::vector<BYTE_T>& commandData, const std::vector<BYTE_T>& commandSignature, int voter_id, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        // Decode EncryptedBallot
        EncryptedBallot ballot;
        std::vector<EncryptedBallotEntry> encryptedBallotEntries;
        std::vector<std::vector<BYTE_T>> encryptedVals;
        std::pair<std::vector<EncryptedBallotEntry>*, std::vector<std::vector<BYTE_T>>*> args = {&encryptedBallotEntries, &encryptedVals};
        ballot.encrypted_ballot_entries.arg = (void*) &args;
        ballot.encrypted_ballot_entries.funcs.decode = &EncryptedBallotEntriesDecodeFunc;
        pb_istream_t pbBuf = pb_istream_from_buffer(&(commandData[0]), commandData.size());
        bool res = pb_decode(&pbBuf, EncryptedBallot_fields, &ballot);
        if(!res) {
                logger.info("Invalid ballot!");
                return errorResponse("Invalid ballot!", logger, config);
        }
        
        pqxx::work txn(dbConn);

        // Get voter group id
        pqxx::result r = txn.exec(
                "SELECT v.voter_group_id"
                " FROM voters v"
                " WHERE v.id = " + std::to_string(voter_id));
        if(r.size() != 1) {
                logger.info("Invalid voter!");
                return errorResponse("Invalid voter!", logger, config);
        }
        int voter_group_id = r[0][0].as<int>();

        // Get current time
        int curr_time = getCurrentTime();

        // Verify that:
        // 1. Election with this ID exists
        // 2. Election has started and has not ended
        // 3. Voter is authorized for this election
        r = txn.exec(
                "SELECT TRUE"
                " FROM elections e"
                " LEFT JOIN ELECTIONS_VOTER_GROUPS evg ON e.id = evg.election_id"
                " WHERE e.id = " + std::to_string(ballot.election_id) +
                " AND e.start_time <= " + std::to_string(curr_time) + 
                " AND e.end_time >= " + std::to_string(curr_time) +
                " AND evg.voter_group_id = " + std::to_string(voter_group_id));
        if(r.size() != 1) {
                logger.info("Invalid election!");
                return errorResponse("Invalid election!", logger, config);
        }

        // Fetch candidate IDs
        r = txn.exec(
                "SELECT c.id"
                " FROM candidates c"
                " WHERE c.election_id = " + std::to_string(ballot.election_id) +
                " ORDER BY c.id");

        // Verify that:
        // 1. There is exactly EncryptedBallotEntry for each candidate, ordered by candidate ID
        // 2. Each EncryptedBallotEntry contains an encrypted 0 or 1 (and there is exactly one 1)
        // 3. There are no extra EncryptedBallotEntries
        bool valid_ballot_entries = true;
        bool foundOne = false;
        if(encryptedBallotEntries.size() != r.size()) {
                valid_ballot_entries = false;
        }
        if(valid_ballot_entries) {
                for(int c_num = 0; c_num < r.size(); c_num++) {
                        const EncryptedBallotEntry& entry = encryptedBallotEntries[c_num];
                
                        if(entry.candidate_id != r[c_num][0].as<int>()) {
                                valid_ballot_entries = false;
                                break;
                        }

                        unsigned long int ptext = -1;
                        paillierDec((char*) &(encryptedVals[c_num][0]), encryptedVals[c_num].size(), &(config.paillierPrivKeyP()[0]), &(config.paillierPrivKeyQ()[0]), &(config.paillierPubKey()[0]), &ptext);
                        if(ptext == 0) {
                                // OK
                        } else if(ptext == 1 && !foundOne) {
                                foundOne = true;
                        } else {
                                valid_ballot_entries = false;
                                break;
                        }
                }
        }
        if(!foundOne) {
                valid_ballot_entries = false;
        }
        if(!valid_ballot_entries) {
                logger.info("Invalid ballot entries!");
                return errorResponse("Invalid ballot entries!", logger, config);
        }

        // Valid ballot.
        // Persist to db
        r = txn.exec(
                "INSERT INTO cast_encrypted_ballots (voter_id, cast_at, election_id, cast_command_data, voter_signature)"
                " VALUES (" + std::to_string(voter_id) + "," + std::to_string(curr_time) + "," + std::to_string(ballot.election_id) + "," + txn.quote(txn.esc_raw(&(commandData[0]), commandData.size())) + "," + txn.quote(txn.esc_raw(&(commandSignature[0]), commandSignature.size())) + ")"
                " RETURNING id");
        for(int c_num = 0; c_num < encryptedBallotEntries.size(); c_num++) {
                EncryptedBallotEntry& entry = encryptedBallotEntries[c_num];
                pqxx::result r2 = txn.exec(
                        "INSERT INTO cast_encrypted_ballot_entries (cast_encrypted_ballot_id, candidate_id, encrypted_value)"
                        " VALUES (" + std::to_string(r[0][0].as<int>()) + "," + std::to_string(entry.candidate_id) + "," + txn.quote(txn.esc_raw(&(encryptedVals[c_num][0]), encryptedVals[c_num].size())) + ")"
                        " RETURNING id");
                entry.id = r2[0][0].as<int>();
                
        }
        txn.commit();
        
        // Create ballot
        CastEncryptedBallot ceb;
        ceb.id = r[0][0].as<int>();
        ceb.voter_id = voter_id;
        ceb.cast_at = curr_time;

        ceb.encrypted_ballot = ballot;
        ceb.encrypted_ballot.encrypted_ballot_entries.arg = &encryptedBallotEntries;
        ceb.encrypted_ballot.encrypted_ballot_entries.funcs.encode = &RepeatedEncryptedBallotEntryEncodeFunc;
        for(int i = 0; i < encryptedBallotEntries.size(); i++) {
                encryptedBallotEntries[i].encrypted_value.arg = &(encryptedVals[i]);
                encryptedBallotEntries[i].encrypted_value.funcs.encode = ByteTArrayEncodeFunc;
        }

        ceb.cast_command_data.arg = (void*) &commandData;
        ceb.cast_command_data.funcs.encode = ByteTArrayEncodeFunc; 
        ceb.voter_signature.arg = (void*) &commandSignature;
        ceb.voter_signature.funcs.encode = ByteTArrayEncodeFunc; 

        // Encode and return response
        Response resp;
        resp.type = ResponseType_CAST_ENCRYPTED_BALLOT;
        std::pair<bool, std::vector<BYTE_T>> cebEncoded = encodeMessage<CastEncryptedBallot>(CastEncryptedBallot_fields, ceb);
        if(!cebEncoded.first) {
                logger.error("Unable to encode!");
                return {false, {}};
        }
        resp.data.arg = (void*) &cebEncoded.second;
        resp.data.funcs.encode = ByteTArrayEncodeFunc; 

        logger.info("Cast ballot complete!");
        return finishResponse(resp, logger, config);
}

std::pair<bool, std::vector<BYTE_T>> getElections(const PaginationMetadata& pagination, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        pqxx::work txn(dbConn);
        pqxx::result r = txn.exec(
                "SELECT e.id, e.start_time, e.end_time, e.enabled, e.allow_write_in"
                " FROM elections e"
                " WHERE e.id > " + std::to_string(pagination.lastId) +
                " ORDER BY e.id ASC"
                " LIMIT " + std::to_string(PAGINATION_MAX));

        std::vector<Election> elections(r.size());
        std::vector<std::vector<int>> authVoterGroups(r.size());
        std::vector<std::vector<Candidate>> candidates(r.size());
        std::vector<std::vector<std::pair<std::string, std::string>>> candidatesNames(r.size());
        std::vector<std::vector<TallyEntry>> tallyEntries(r.size());
        std::vector<std::vector<std::pair<std::vector<BYTE_T>, std::vector<BYTE_T>>>> tallyEntriesData(r.size());
        for(int i = 0; i < r.size(); i++) {
                elections[i].id = r[i][0].as<int>();
                elections[i].start_time_utc = r[i][1].as<int>();
                elections[i].end_time_utc = r[i][2].as<int>();
                elections[i].enabled = r[i][3].as<bool>();
                elections[i].allow_write_in = r[i][4].as<bool>();

                pqxx::result tally_r = txn.exec(
                        "SELECT te.tally_id, te.id, te.candidate_id, te.encrypted_value, te.encryption_r, te.decrypted_value"
                        " FROM tally_entries te"
                        " LEFT JOIN tallies t on t.id = te.tally_id"
                        " WHERE t.election_id = " + std::to_string(elections[i].id) +
                        " ORDER BY te.candidate_id ASC");
                if(tally_r.size() == 0) {
                        elections[i].tally.finalized = false;
                        elections[i].tally.tally_entries.funcs.encode = NULL;
                } else {
                        elections[i].tally.id = tally_r[0][0].as<int>();
                        elections[i].tally.finalized = true;
        
                        tallyEntries[i].resize(tally_r.size());
                        tallyEntriesData[i].resize(tally_r.size());
                
                        for(int j = 0; j < tally_r.size(); j++) {
                                if(tally_r[j][0].as<int>() != elections[i].tally.id) {
                                        logger.error("Invalid election tally - multiple tally IDs!");
                                        return {false, {}};
                                }
        
                                tallyEntries[i][j].id = tally_r[j][1].as<int>();
                                tallyEntries[i][j].candidate_id = tally_r[j][2].as<int>();

                                pqxx::binarystring encryptedVal(tally_r[j][3]);
                                tallyEntriesData[i][j].first.resize(encryptedVal.size());
                                memcpy(&(tallyEntriesData[i][j].first[0]), encryptedVal.data(), encryptedVal.size());
                                tallyEntries[i][j].encrypted_value.arg = &(tallyEntriesData[i][j].first);
                                tallyEntries[i][j].encrypted_value.funcs.encode = ByteTArrayEncodeFunc;

                                pqxx::binarystring encryptionR(tally_r[j][4]);
                                tallyEntriesData[i][j].second.resize(encryptionR.size());
                                memcpy(&(tallyEntriesData[i][j].second[0]), encryptionR.data(), encryptionR.size());
                                tallyEntries[i][j].encryption_r.arg = &(tallyEntriesData[i][j].second);
                                tallyEntries[i][j].encryption_r.funcs.encode = ByteTArrayEncodeFunc;

                                tallyEntries[i][j].decrypted_value = tally_r[j][5].as<int>();
                        }

                        elections[i].tally.tally_entries.arg = &(tallyEntries[i]);
                        elections[i].tally.tally_entries.funcs.encode = RepeatedTallyEntryEncodeFunc;
                }
                
                
                pqxx::result evg_r = txn.exec(
                        "SELECT evg.id"
                        " FROM elections_voter_groups evg"
                        " WHERE evg.id = " + std::to_string(elections[i].id) +
                        " ORDER BY evg.id ASC");

                authVoterGroups[i].resize(evg_r.size());
                for(int j = 0; j < evg_r.size(); j++) {
                        authVoterGroups[i][j] = evg_r[j][0].as<int>();
                }
                elections[i].authorized_voter_group_ids.arg = &(authVoterGroups[i]);
                elections[i].authorized_voter_group_ids.funcs.encode = IntArrayEncodeFunc;

                pqxx::result cand_r = txn.exec(
                        "SELECT c.id, c.first_name, c.last_name"
                        " FROM candidates c"
                        " WHERE c.election_id = " + std::to_string(elections[i].id) +
                        " ORDER BY c.id ASC");

                candidates[i].resize(cand_r.size());
                candidatesNames[i].resize(cand_r.size());
                for(int j = 0; j < cand_r.size(); j++) {
                        int returned_id = cand_r[j][0].as<int>();
                        std::string returned_fname = cand_r[j][1].as<std::string>();
                        std::string returned_lname = cand_r[j][2].as<std::string>();

                        candidates[i][j].id = returned_id;

                        candidatesNames[i][j].first = returned_fname;
                        candidates[i][j].first_name.arg = &(candidatesNames[i][j].first);
                        candidates[i][j].first_name.funcs.encode = StringEncodeFunc;
        
                        candidatesNames[i][j].second = returned_lname;
                        candidates[i][j].last_name.arg = &(candidatesNames[i][j].second);
                        candidates[i][j].last_name.funcs.encode = StringEncodeFunc;
                }

                elections[i].candidates.arg = &(candidates[i]);
                elections[i].candidates.funcs.encode = RepeatedCandidateEncodeFunc;
        }

        // Prepare elections obj for encoding
        Elections electionsObj;
        electionsObj.elections.arg = (void*) &elections;
        electionsObj.elections.funcs.encode = RepeatedElectionEncodeFunc;

        // Encode and return response
        Response resp;
        resp.type = ResponseType_ELECTIONS;
        std::pair<bool, std::vector<BYTE_T>> electionsEncoded = encodeMessage<Elections>(Elections_fields, electionsObj);
        if(!electionsEncoded.first) {
                logger.error("Unable to encode elections!");
                return {false, {}};
        }

        resp.data.arg = (void*) &(electionsEncoded.second);
        resp.data.funcs.encode = ByteTArrayEncodeFunc; 

        logger.info("Get elections complete!");
        return finishResponse(resp, logger, config);
}

std::pair<bool, std::vector<BYTE_T>> getVoters(const PaginationMetadata& pagination, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        pqxx::work txn(dbConn);
        pqxx::result r = txn.exec(
                "SELECT v.id, v.voter_group_id, v.smartcard_public_key"
                " FROM voters v"
                " WHERE v.id > " + std::to_string(pagination.lastId) +
                " ORDER BY v.id ASC"
                " LIMIT " + std::to_string(PAGINATION_MAX));

        std::vector<Voter> voters(r.size());
        std::vector<std::vector<BYTE_T>> pubKeys(r.size());
        for(int i = 0; i < r.size(); i++) {
                voters[i].id = r[i][0].as<int>();
                voters[i].voter_group_id = r[i][1].as<int>();

                pqxx::binarystring pubKeyData(r[i][2]);
                pubKeys[i].resize(pubKeyData.size());
                memcpy(&(pubKeys[i][0]), pubKeyData.data(), pubKeyData.size());
                voters[i].pubkey.arg = &(pubKeys[i]);
                voters[i].pubkey.funcs.encode = ByteTArrayEncodeFunc;
        }

        // Prepare Voters object for encoding
        Voters votersObj;
        votersObj.voters.arg = (void*) &voters;
        votersObj.voters.funcs.encode = RepeatedVoterEncodeFunc;

        // Encode and return response
        Response resp;
        resp.type = ResponseType_VOTERS;
        std::pair<bool, std::vector<BYTE_T>> votersEncoded = encodeMessage<Voters>(Voters_fields, votersObj);
        if(!votersEncoded.first) {
                logger.error("Unable to encode voters!");
                return {false, {}};
        }

        resp.data.arg = (void*) &(votersEncoded.second);
        resp.data.funcs.encode = ByteTArrayEncodeFunc; 

        logger.info("Get voters complete!");
        return finishResponse(resp, logger, config);
}

std::pair<bool, std::vector<BYTE_T>> getEncryptedBallots(const CastEncryptedBallotsRequest& request, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        pqxx::work txn(dbConn);
        pqxx::result r = txn.exec(
                "SELECT ceb.id, ceb.voter_id, ceb.cast_at, ceb.cast_command_data, ceb.voter_signature"
                " FROM cast_encrypted_ballots ceb"
                " WHERE ceb.id > " + std::to_string(request.pagination_metadata.lastId) +
                " AND ceb.election_id = " + std::to_string(request.election_id) +
                " ORDER BY ceb.id ASC"
                " LIMIT " + std::to_string(PAGINATION_MAX));

        std::vector<CastEncryptedBallot> cebs(r.size());
        std::vector<std::pair<std::vector<BYTE_T>, std::vector<BYTE_T>>> cebsData(r.size());
        std::vector<std::vector<EncryptedBallotEntry>> ebes(r.size());
        std::vector<std::vector<std::vector<BYTE_T>>> encryptedVals(r.size());
        for(int i = 0; i < r.size(); i++) {
                cebs[i].id = r[i][0].as<int>();
                cebs[i].voter_id = r[i][1].as<int>();
                cebs[i].cast_at = r[i][2].as<int>();

                pqxx::binarystring castCommandData(r[i][3]);
                cebsData[i].first.resize(castCommandData.size());
                memcpy(&(cebsData[i].first[0]), castCommandData.data(), castCommandData.size());
                cebs[i].cast_command_data.arg = &(cebsData[i].first);
                cebs[i].cast_command_data.funcs.encode = ByteTArrayEncodeFunc;

                pqxx::binarystring voterSignatureData(r[i][4]);
                cebsData[i].second.resize(voterSignatureData.size());
                memcpy(&(cebsData[i].second[0]), voterSignatureData.data(), voterSignatureData.size());
                cebs[i].voter_signature.arg = &(cebsData[i].second);
                cebs[i].voter_signature.funcs.encode = ByteTArrayEncodeFunc;
                
                pqxx::result cebe_r = txn.exec(
                        "SELECT cebe.id, cebe.candidate_id, cebe.encrypted_value"
                        " FROM cast_encrypted_ballot_entries cebe"
                        " WHERE cebe.cast_encrypted_ballot_id = " + std::to_string(cebs[i].id) +
                        " ORDER BY cebe.candidate_id ASC");
                ebes[i].resize(cebe_r.size());
                encryptedVals[i].resize(cebe_r.size());
                
                for(int j = 0; j < cebe_r.size(); j++) {
                        ebes[i][j].id = cebe_r[j][0].as<int>(); 
                        ebes[i][j].candidate_id = cebe_r[j][1].as<int>(); 

                        pqxx::binarystring encryptedVal(cebe_r[j][2]);
                        encryptedVals[i][j].resize(encryptedVal.size());
                        memcpy(&(encryptedVals[i][j][0]), encryptedVal.data(), encryptedVal.size());
                        ebes[i][j].encrypted_value.arg = &(encryptedVals[i][j]);
                        ebes[i][j].encrypted_value.funcs.encode = ByteTArrayEncodeFunc;
                } 

                cebs[i].encrypted_ballot.election_id = request.election_id;
                cebs[i].encrypted_ballot.encrypted_ballot_entries.arg = &(ebes[i]);
                cebs[i].encrypted_ballot.encrypted_ballot_entries.funcs.encode = RepeatedEncryptedBallotEntryEncodeFunc;
        }

        // Prepare CastEncryptedBallots object for encoding
        CastEncryptedBallots cebesObj;
        cebesObj.cast_encrypted_ballots.arg = (void*) &cebs;
        cebesObj.cast_encrypted_ballots.funcs.encode = RepeatedCastEncryptedBallotEncodeFunc;

        // Encode and return response
        Response resp;
        resp.type = ResponseType_CAST_ENCRYPTED_BALLOTS;
        std::pair<bool, std::vector<BYTE_T>> cebesEncoded = encodeMessage<CastEncryptedBallots>(CastEncryptedBallots_fields, cebesObj);
        if(!cebesEncoded.first) {
                logger.error("Unable to encode cebes!");
                return {false, {}};
        }

        resp.data.arg = (void*) &(cebesEncoded.second);
        resp.data.funcs.encode = ByteTArrayEncodeFunc; 

        logger.info("Get cast ballots complete!");
        return finishResponse(resp, logger, config);
}

std::pair<bool, std::vector<BYTE_T>> processCommand(const std::vector<BYTE_T>& command, pqxx::connection& dbConn, Logger& logger, const Config& config) {
        logger.info("Processing command");

        // Prepare command for parsing
        std::vector<BYTE_T> commandData;
        std::vector<BYTE_T> commandPubKey;
        std::vector<BYTE_T> commandSignature;
        Command commandParsed;
        commandParsed.data.arg = (void*) &commandData;
        commandParsed.data.funcs.decode = ByteTArrayDecodeFunc;
        commandParsed.pubkey.arg = (void*) &commandPubKey;
        commandParsed.pubkey.funcs.decode = ByteTArrayDecodeFunc;
        commandParsed.signature.arg = (void*) &commandSignature;
        commandParsed.signature.funcs.decode = ByteTArrayDecodeFunc;

        // Parse Command
        pb_istream_t pbBuf = pb_istream_from_buffer(&(command[0]), command.size());
        bool res = pb_decode(&pbBuf, Command_fields, &commandParsed);
        if(!res) {
                logger.info("Invalid command!");
                return errorResponse("Invalid Command!", logger, config);
        }
        pb_istream_t dataBuf = pb_istream_from_buffer(&(commandData[0]), commandData.size());

        // Check if public key is in database
        int voter_id;
        {
                pqxx::work txn(dbConn);
                pqxx::result r = txn.exec(
                        "SELECT id"
                        " FROM VOTERS"
                        " WHERE SMARTCARD_PUBLIC_KEY = " + txn.quote(txn.esc_raw(&(commandPubKey[0]), commandPubKey.size())));
                if(r.size() != 1) {
                        logger.info("Invalid voter!");
                        return errorResponse("Invalid Voter!", logger, config);
                }

                voter_id = r[0][0].as<int>();
        }

        // Validate signature
        int commandTypeAndDataLen = sizeof(commandParsed.type) + commandData.size();
        unsigned char commandTypeAndData[commandTypeAndDataLen];
        memcpy(
                &commandTypeAndData, 
                &commandParsed.type, 
                sizeof(commandParsed.type));
        memcpy(
                commandTypeAndData + sizeof(commandParsed.type), 
                &(commandData[0]), 
                commandData.size());
        bool validSig = rsaVerify(
                commandTypeAndData, 
                commandTypeAndDataLen, 
                &(commandSignature[0]),
                commandSignature.size(),
                &(commandPubKey[0]),
                commandPubKey.size());
        if(!validSig) {
                logger.info("Invalid signature!");
                return errorResponse("Invalid signature!", logger, config);
        }
        
        // TODO: Handle each command type
        switch(commandParsed.type) {
                case(CommandType_GET_ELECTIONS):
                {
                        PaginationMetadata pagination;
                        bool res = pb_decode(&dataBuf, PaginationMetadata_fields, &pagination);

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
                        PaginationMetadata pagination;
                        bool res = pb_decode(&dataBuf, PaginationMetadata_fields, &pagination);

                        if(!res) {
                                logger.info("Invalid pagination data!");
                                return errorResponse("Invalid pagination data!", logger, config);
                        }

                        return getVoters(pagination, dbConn, logger, config);
                }
                case(CommandType_GET_PLAINTEXT_BALLOTS):
                {
                }
                case(CommandType_GET_ENCRYPTED_BALLOTS):
                {
                        CastEncryptedBallotsRequest cebRequest;
                        bool res = pb_decode(&dataBuf, CastEncryptedBallotsRequest_fields, &cebRequest);

                        if(!res) {
                                logger.info("Invalid request data!");
                                return errorResponse("Invalid request data!", logger, config);
                        }

                        return getEncryptedBallots(cebRequest, dbConn, logger, config);
                }
                case(CommandType_GET_PLAINTEXT_BALLOT):
                {
                }
                case(CommandType_GET_ENCRYPTED_BALLOT):
                {
                }
                case(CommandType_CAST_BALLOT):
                {
                        return castBallot(commandData, commandSignature, voter_id, dbConn, logger, config);
                }
                default:
                {
                        logger.info("Invalid Command!");
                        return errorResponse("Invalid Command!", logger, config);
                }
        }
}
