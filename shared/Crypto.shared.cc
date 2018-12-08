#include "Crypto.shared.h"

#include <cryptopp/eccrypto.h>
#include <cryptopp/osrng.h>
#include <cryptopp/oids.h>

using namespace CryptoPP;

void GenerateKeyPair(std::string& privateKeyStr, std::string& publicKeyStr) {
        // Generate private key
        AutoSeededRandomPool prng;
        ECDSA<ECP, SHA1>::PrivateKey privateKey;
        privateKey.Initialize(prng, ASN1::secp160r1()); // TODO: use better curve
        bool result = privateKey.Validate(prng, 3);
        if(!result) {
                throw CryptoError("Unable to validate generated privkey!");
        }

        // Generate public key
        ECDSA<ECP, SHA1>::PublicKey publicKey;
        privateKey.MakePublicKey(publicKey);
        result = publicKey.Validate(prng, 3);
        if(!result) {
                throw CryptoError("Unable to validate generated pubkey!");
        }
        
        // Export private, public key to strings
        StringSink privateKeySink(privateKeyStr);
        privateKey.Save(privateKeySink);
        StringSink publicKeySink(publicKeyStr);
        publicKey.Save(publicKeySink);
}

void SignMessage() {
}

void VerifyMessage() {
}
