#pragma once

#include "shared_c/crypto/Cryptography.h"
#include "wolfssl/options.h"
#include "wolfssl/wolfcrypt/coding.h"
#include "wolfssl/wolfcrypt/rsa.h"
#include <vector>
#include <string>
#include <iostream>

#include "shared_c/Definitions.h"

class Config {
public:
        Config(const char* db_user, const char* db_pass, const char* db_host, const char* db_port, const char* db_name, const char* db_migrations, const char* server_pubkey_base64, const char* server_paillier_pubkey_hex, const char* client_privkey_base64, const char* server_host, const char* server_port) {
                _valid = true;

                if(db_user == NULL || db_pass == NULL || db_host == NULL || db_port == NULL || db_name == NULL || db_migrations == NULL || server_pubkey_base64 == NULL || server_paillier_pubkey_hex == NULL || client_privkey_base64 == NULL || server_host == NULL || server_port == NULL) {
                        _valid = false;
                        return;
                }

                _db_user = db_user;
                _db_pass = db_pass;
                _db_host = db_host;
                _db_name = db_name;
                _db_migrations = db_migrations;
                _server_host = server_host;
                _server_port = std::stoi(server_port);
                _serverPaillierPubkey = server_paillier_pubkey_hex;

                // Decode client privkey
                // Length comes from wolfssl docs
                _clientPrivKey.resize(RSA_PRIVATE_KEY_SIZE_DER);
                unsigned int privKeySize = _clientPrivKey.size();
                int res = Base64_Decode(
                        (const byte*) client_privkey_base64,
                        strlen(client_privkey_base64),
                        &_clientPrivKey[0],
                        &privKeySize
                );
                if(res != 0) {
                        _valid = false;
                        return;
                }
                _clientPrivKey.resize(privKeySize);

                // Calculate client pubkey
                unsigned int idx = 0;
                RsaKey key;
                wc_InitRsaKey(&key, NULL);
                res = wc_RsaPrivateKeyDecode(
                        &_clientPrivKey[0],
                        &idx,
                        &key,
                        _clientPrivKey.size()
                );
                if(res != 0) {
                        wc_FreeRsaKey(&key);
                        _valid = false;
                        return;
                }

                // TODO: research exactly how long pubkey should be
                _clientPubKey.resize(_clientPrivKey.size());
                res = wc_RsaKeyToPublicDer(
                        &key,
                        &_clientPubKey[0],
                        _clientPubKey.size()
                );
                wc_FreeRsaKey(&key);
                if(res <= 0) {
                        _valid = false;
                        return;
                }
                _clientPubKey.resize(res);

                // Decode server pubkey 
                // Length comes from wolfssl docs
                _serverPubKey.resize(RSA_PUBLIC_KEY_SIZE_DER);
                unsigned int pubKeySize = _serverPubKey.size();
                res = Base64_Decode(
                        (const byte*) server_pubkey_base64,
                        strlen(server_pubkey_base64),
                        &_serverPubKey[0],
                        &pubKeySize
                );
                if(res != 0) {
                        _valid = false;
                        return;
                }
                _serverPubKey.resize(pubKeySize);
        }

        bool valid() const { return _valid; }

        const std::string& dbUser() const { return _db_user; }
        const std::string& dbPass() const { return _db_pass; }
        const std::string& dbHost() const { return _db_host; }
        const std::string& dbPort() const { return _db_port; }
        const std::string& dbName() const { return _db_name; }
        const std::string& dbMigrations() const { return _db_migrations; }
        const std::string& serverHost() const { return _server_host; }
        int serverPort() const { return _server_port; }

        const std::vector<BYTE_T>& serverPubKey() const { return _serverPubKey; }
        const std::vector<BYTE_T>& clientPrivKey() const { return _clientPrivKey; }
        const std::vector<BYTE_T>& clientPubKey() const { return _clientPubKey; }

        // Return copy because lib requires mutating access
        std::string serverPaillierPubKey() const { return _serverPaillierPubkey; }

private:
        bool _valid;

        std::string _db_user;
        std::string _db_pass;
        std::string _db_host;
        std::string _db_port;
        std::string _db_name;
        std::string _db_migrations;
        std::string _server_host;
        int _server_port;

        std::vector<BYTE_T> _serverPubKey;
        std::vector<BYTE_T> _clientPrivKey;
        std::vector<BYTE_T> _clientPubKey;

        std::string _serverPaillierPubkey;
};
