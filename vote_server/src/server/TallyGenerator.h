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
        std::unordered_map<int, std::vector<pqxx::binarystring>> ctexts;
        for(int idx = 0; idx < r.size(); idx++) {
                int cid = r[idx][0].as<int>();

                if(ctexts.find(cid) == ctexts.end()) {
                        ctexts[cid] = std::vector<pqxx::binarystring>();
                } 
                ctexts[cid].emplace_back(r[idx][1]);
        }

        // Calculate encrypted tallies for each candidate
        std::unordered_map<int, char*> tallies;
        for(auto const& it : ctexts) {
                // Get ctext ptrs/sizes for this candidate 
                char* ctextPtrs[it.second.size()];
                int ctextSizes[it.second.size()];
                for(int i = 0; i < it.second.size(); i++) {
                        ctextPtrs[i] = it.second[i].data();
                        ctextSizes[i] = it.second[i].size();
                }

                // Calculate encrypted tally for this candidate
                tallies[it.first] = NULL;
                paillierSum(
                        &(tallies[it.first]),
                        ctextPtrs,
                        ctextSizes,
                        it.second.size(),
                        config.paillierPubKey().c_str()
                );
        }

        // Calculate decrypted tallies for each candidate
        // TODO: get encryption r
        std::unordered_map<int, std::pair<unsigned long int, int>> talliesDec;
        for(auto const& it : tallies) {
                talliesDec[it.first] = -1;
                paillierDec(
                        it.second, 
                        P_CIPHERTEXT_MAX_LEN,
                        config.paillierPrivKey().c_str()
                        config.paillierPubKey().c_str()
                        &(talliesDec[it.first][0])
                );
                paillierGetRand(
                        it.second,
                        P_CIPHERTEXT_MAX_LEN,
                        config.paillierPrivKey().c_str(),
                        config.paillierPubKey().c_str(),
                        &(talliesDec[it.first][1])
                );
        }

        // Save decrypted tallies for each candidate
        r = txn.exec("INSERT INTO tallies (election_id) VALUES (" + std::to_string(electionId) + ") RETURNING id");
        for(auto const& it : talliesDec) {
                txn.exec("INSERT INTO tally_entries"
                         " (tally_id, candidate_id, encrypted_value, decrypted_value, encryption_r)"
                         " VALUES (" std::to_string(r[0][0].as<int>() + "," + std::to_string(it.first) + "," + txn.quote(txn.esc_raw(tallies[it], P_CIPHERTEXT_MAX_LEN)) + "," + std::to_string(it.second[0]) + "," + std::to_string(it.second[1]) + ")"));
        }
}
