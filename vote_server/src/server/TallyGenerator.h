#include <unordered_map>

#include "shared_cpp/logger/Logger.h"
#include "shared_cpp/database/Database.h"

// TODOs not related to this method:
// TODO: improve exception handling in general
// TODO: fix replay attack possibility
// TODO: fix signatures (don't just sign hash?)
// TODO: fix ability to cast multiple ballots
void generateTally(int electionId, pqxx::work& txn, Logger& logger, const Config& config) {
        // Get list of ctexts for each candidate
        pqxx::result r = txn.exec(
                "SELECT cebe.candidate_id, cebe.encrypted_value"
                " FROM cast_encrypted_ballot_entries cebe"
                " LEFT JOIN candidates c ON c.id = cebe.candidate_id"
                " WHERE c.election_id = " + std::to_string(electionId));
        std::unordered_map<int, std::vector<std::vector<BYTE_T>>> ctexts;
        for(int idx = 0; idx < r.size(); idx++) {
                int cid = r[idx][0].as<int>();

                if(ctexts.find(cid) == ctexts.end()) {
                        ctexts[cid] = std::vector<std::vector<BYTE_T>>();
                } 

                pqxx::binarystring bstr(r[idx][1]);
                ctexts[cid].emplace_back(bstr.size());
                memcpy(&(ctexts[cid][0]), bstr.data(), bstr.size());
        }

        // Calculate encrypted tallies for each candidate
        std::unordered_map<int, char*> tallies;
        for(auto& it : ctexts) {
                // Get ctext ptrs/sizes for this candidate 
                BYTE_T* ctextPtrs[it.second.size()];
                int ctextSizes[it.second.size()];
                for(int i = 0; i < it.second.size(); i++) {
                        ctextPtrs[i] = &(it.second[i][0]);
                        ctextSizes[i] = it.second[i].size();
                }

                // Calculate encrypted tally for this candidate
                tallies[it.first] = NULL;
                paillierSum(
                        (void**) &(tallies[it.first]),
                        (char**) ctextPtrs,
                        ctextSizes,
                        it.second.size(),
                        &(config.paillierPubKey()[0])
                );
        }

        // Calculate decrypted tallies for each candidate
        // TODO: make Crypto use BYTE_T instead of char? (consistent typing)
        std::unordered_map<int, std::pair<unsigned long int, char*>> talliesDec;
        for(auto const& it : tallies) {
                talliesDec[it.first] = {-1, NULL};
                paillierDec(
                        it.second, 
                        P_CIPHERTEXT_MAX_LEN,
                        &(config.paillierPrivKeyP()[0]),
                        &(config.paillierPrivKeyQ()[0]),
                        &(config.paillierPubKey()[0]),
                        &(talliesDec[it.first].first)
                );
                paillierGetRand(
                        it.second,
                        P_CIPHERTEXT_MAX_LEN,
                        &(config.paillierPrivKeyP()[0]),
                        &(config.paillierPrivKeyQ()[0]),
                        &(config.paillierPubKey()[0]),
                        &(talliesDec[it.first].second)
                );
        }

        // Save decrypted tallies for each candidate
        r = txn.exec("INSERT INTO tallies (election_id) VALUES (" + std::to_string(electionId) + ") RETURNING id");
        for(auto const& it : talliesDec) {
                txn.exec("INSERT INTO tally_entries"
                         " (tally_id, candidate_id, encrypted_value, decrypted_value, encryption_r)"
                         " VALUES (" + std::to_string(r[0][0].as<int>()) + "," + std::to_string(it.first) + "," + txn.quote(txn.esc_raw((BYTE_T*) tallies[it.first], P_CIPHERTEXT_MAX_LEN)) + "," + std::to_string(it.second.first) + "," + txn.quote(txn.esc_raw((BYTE_T*) it.second.second, strlen(it.second.second) + 1)) + ")");
        }

        for(auto const& it : tallies) {
                free(it.second);
                free(talliesDec[it.first].second);
        }
}
