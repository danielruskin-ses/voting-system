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
                memcpy(&(ctexts[cid][idx][0]), bstr.data(), bstr.size());
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
        std::unordered_map<int, std::tuple<unsigned long int, char*, long unsigned int>> talliesDec;
        for(auto const& it : tallies) {
                talliesDec[it.first] = {0, NULL, 0};
                char* ptext;
                paillierDec(
                        it.second, 
                        P_CIPHERTEXT_MAX_LEN,
                        &(config.paillierPrivKeyP()[0]),
                        &(config.paillierPrivKeyQ()[0]),
                        &(config.paillierPubKey()[0]),
                        sizeof(unsigned long int),
                        &ptext
                );
                std::get<0>(talliesDec[it.first]) = *((unsigned long int*) ptext);
                free(ptext);

                paillierGetRand(
                        it.second,
                        P_CIPHERTEXT_MAX_LEN,
                        &(config.paillierPrivKeyP()[0]),
                        &(config.paillierPrivKeyQ()[0]),
                        &(config.paillierPubKey()[0]),
                        &(std::get<1>(talliesDec[it.first])),
                        &(std::get<2>(talliesDec[it.first]))
                );
        }

	// Pull all encrypted write-in ballots for this candidate
	std::vector<std::pair<mpz_t, mpz_t>> writeInBallots;
        r = txn.exec(
                "SELECT ceb.encrypted_write_in_a, ceb.encrypted_write_in_b"
                " FROM cast_encrypted_ballots ceb"
                " WHERE ceb.election_id = " + std::to_string(electionId));
        for(int idx = 0; idx < r.size(); idx++) {
                pqxx::binarystring aval(r[idx][0]);
                pqxx::binarystring bval(r[idx][1]);

                writeInBallots.emplace_back();
		mpz_init(writeInBallots.back().first);
		mpz_import(writeInBallots.back().first, aval.size(), 1, 1, 0, 0, aval.data());
		mpz_init(writeInBallots.back().second);
		mpz_import(writeInBallots.back().second, bval.size(), 1, 1, 0, 0, bval.data());
        }

	// Shuffle encrypted write-in ballots
	std::vector<std::pair<mpz_t, mpz_t>> shuffleOut;
	std::vector<BYTE_T> proofOut;
	elGamalShuffle(
		(const char*) &(config.vtmfGroup()[0]),
		config.vtmfGroup().size(),
		(const char*) &(config.vtmfKey()[0]),
		config.vtmfKey().size(),
		writeInBallots,
		shuffleOut,
		proofOut
	);

	// Persist everything to db
        r = txn.exec("INSERT INTO tallies (election_id, shuffle_proof) VALUES (" + std::to_string(electionId) + "," + txn.quote(txn.esc_raw(&(proofOut[0]), proofOut.size())) + ") RETURNING id");
        for(auto const& it : talliesDec) {
                txn.exec("INSERT INTO tally_entries"
                         " (tally_id, candidate_id, encrypted_value, decrypted_value, encryption_r)"
                         " VALUES (" + std::to_string(r[0][0].as<int>()) + "," + std::to_string(it.first) + "," + txn.quote(txn.esc_raw((BYTE_T*) tallies[it.first], P_CIPHERTEXT_MAX_LEN)) + "," + std::to_string(std::get<0>(it.second)) + "," + txn.quote(txn.esc_raw((BYTE_T*) std::get<1>(it.second), std::get<2>(it.second))) + ")");
        }
	
	for(auto const& it : shuffleOut) {
		std::vector<BYTE_T> aExport = exportMpz(it.first);
		std::vector<BYTE_T> bExport = exportMpz(it.second);
		char* decExport;
		unsigned int decExportLen;
		elGamalDecrypt(
			(const char*) &(config.vtmfGroup()[0]),
			config.vtmfGroup().size(),
			(const char*) &(config.vtmfX()[0]),
			config.vtmfX().size(),
			(const char*) &(aExport[0]),
			aExport.size(),
			(const char*) &(bExport[0]),
			bExport.size(),
			&decExport,
			&decExportLen
		);

                txn.exec("INSERT INTO write_in_tally_entries"
                         " (tally_id, encrypted_value_a, encrypted_value_b, decrypted_value)"
                         " VALUES (" + std::to_string(r[0][0].as<int>()) + "," + txn.quote(txn.esc_raw(&(aExport[0]), aExport.size())) + "," + txn.quote(txn.esc_raw(&(bExport[0]), bExport.size())) + "," + txn.quote(txn.esc_raw((const unsigned char*) decExport, decExportLen)) + ")");

		free(decExport);
	}

	txn.commit();
	
        for(auto& it : tallies) {
                free(it.second);
                free(std::get<1>(talliesDec[it.first]));
        }
	for(auto& it : writeInBallots) {
		mpz_clear(it.first);
		mpz_clear(it.second);
	}
	for(auto& it : shuffleOut) {
		mpz_clear(it.first);
		mpz_clear(it.second);
	}
}
