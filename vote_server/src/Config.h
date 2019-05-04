#pragma once

#include "shared_c/crypto/Cryptography.h"
#include "wolfssl/options.h"
#include "wolfssl/wolfcrypt/coding.h"
#include "wolfssl/wolfcrypt/rsa.h"
#include <string>
#include <iostream>

class Config {
public:
        Config(const char* num_threads, const char* db_user, const char* db_pass, const char* db_host, const char* db_port, const char* db_name, const char* db_migrations, const char* privkey_base64, const char* paillier_pubkey_hex, const char* paillier_privkey_p_hex, const char* paillier_privkey_q_hex, const char* vtmf_key, const char* vtmf_x, const char* vtmf_group) {
                _valid = true;

                if(num_threads == NULL || db_user == NULL || db_pass == NULL || db_host == NULL || db_port == NULL || db_name == NULL || db_migrations == NULL || privkey_base64 == NULL || paillier_pubkey_hex == NULL || paillier_privkey_p_hex == NULL || paillier_privkey_q_hex == NULL || vtmf_g == NULL || vtmf_h == NULL, || vtmf_p == NULL || vtmf_x == NULL || vtmf_group == NULL) {
                        _valid = false;
                        return;
                }

                _num_threads = std::stoi(num_threads);

                _db_user = db_user;
                _db_pass = db_pass;
                _db_host = db_host;
                _db_port = db_port;
                _db_name = db_name;
                _db_migrations = db_migrations;

                unsigned int decodeSize;
                _vtmf_key.resize((sizeof(vtmf_key) * 3 + 3) / 4);
                int res = Base64_Decode(
                        (const byte*) vtmf_key,  
                        strlen(vtmf_key),
                        &(_vtmf_key[0]),
                        &decodeSize
                );
                if(res != 0) {
                        _valid = false;
                        return;
                }
                _vtmf_key.resize(decodeSize);

                _vtmf_x.resize((sizeof(vtmf_x) * 3 + 3) / 4);
                int res = Base64_Decode(
                        (const byte*) vtmf_x,  
                        strlen(vtmf_x),
                        &(_vtmf_x[0]),
                        &decodeSize
                );
                if(res != 0) {
                        _valid = false;
                        return;
                }
                _vtmf_x.resize(decodeSize);

                _vtmf_group.resize((sizeof(vtmf_group) * 3 + 3) / 4);
                int res = Base64_Decode(
                        (const byte*) vtmf_group,  
                        strlen(vtmf_group),
                        &(_vtmf_group[0]),
                        &decodeSize
                );
                if(res != 0) {
                        _valid = false;
                        return;
                }
                _vtmf_group.resize(decodeSize);

                // Decode privkey
                // Length comes from wolfssl docs
                _privKey.resize(RSA_PRIVATE_KEY_SIZE_DER);
                unsigned int privKeySize = _privKey.size();
                res = Base64_Decode(
                        (const byte*) privkey_base64,
                        strlen(privkey_base64),
                        &_privKey[0],
                        &privKeySize
                );
                if(res != 0) {
                        _valid = false;
                        return;
                }
                _privKey.resize(privKeySize);

                // Calculate pubkey
                unsigned int idx = 0;
                RsaKey key;
                wc_InitRsaKey(&key, NULL);
                res = wc_RsaPrivateKeyDecode(
                        &_privKey[0],
                        &idx,
                        &key,
                        _privKey.size()
                );
                if(res != 0) {
                        wc_FreeRsaKey(&key);
                        _valid = false;
                        return;
                }

                // Write RSA pubkey to byte arr
                _pubKey.resize(RSA_PUBLIC_KEY_SIZE_DER);
                res = wc_RsaKeyToPublicDer(
                        &key,
                        &_pubKey[0],
                        _pubKey.size()
                );
                wc_FreeRsaKey(&key);
                if(res <= 0) {
                        _valid = false;
                        return;
                }
                _pubKey.resize(res);

                _paillierPubKey = std::string(paillier_pubkey_hex);
                _paillierPrivKeyP = std::string(paillier_privkey_p_hex);
                _paillierPrivKeyQ = std::string(paillier_privkey_q_hex);
        }

        bool valid() const { return _valid; }

        int numThreads() const { return _num_threads; }

        const std::string& dbUser() const { return _db_user; }
        const std::string& dbPass() const { return _db_pass; }
        const std::string& dbHost() const { return _db_host; }
        const std::string& dbPort() const { return _db_port; }
        const std::string& dbName() const { return _db_name; }
        const std::string& dbMigrations() const { return _db_migrations; }

        const std::vector<BYTE_T>& privKey() const { return _privKey; }
        const std::vector<BYTE_T>& pubKey() const { return _pubKey; }

        // return copy because caller needs mutating access
        std::string paillierPrivKeyP() const { return _paillierPrivKeyP; }
        std::string paillierPrivKeyQ() const { return _paillierPrivKeyQ; }
        std::string paillierPubKey() const { return _paillierPubKey; }

        const std::vector<BYTE_T>& vtmfKey() const { return _vtmf_key; }
        const std::vector<BYTE_T>& vtmfX() const { return _vtmf_x; }
        const std::vector<BYTE_T>& vtmfGroup() const { return _vtmf_group; }

private:
        bool _valid;

        int _num_threads;

        std::vector<BYTE_T> _vtmf_key;
        std::vector<BYTE_T> _vtmf_x;
        std::vector<BYTE_T> _vtmf_group;

        std::string _db_user;
        std::string _db_pass;
        std::string _db_host;
        std::string _db_port;
        std::string _db_name;
        std::string _db_migrations;
        
        std::vector<BYTE_T> _privKey;
        std::vector<BYTE_T> _pubKey;
        std::string _paillierPrivKeyP;
        std::string _paillierPrivKeyQ;
        std::string _paillierPubKey;
};
