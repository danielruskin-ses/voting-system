#include "Crypto.shared.h"

void GenerateKeyPair(std::string& publicKeyStr, std::string& privateKeyStr) {
        // Generate private key
        CryptoPP::AutoSeededRandomPool prng;
        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey privateKey;
        privateKey.Initialize(prng, CryptoPP::ASN1::secp160r1()); // TODO: use better curve
        bool result = privateKey.Validate(prng, 3);
        if(!result) {
                throw std::runtime_error("Unable to validate generated privkey!");
        }

        // Generate public key
        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PublicKey publicKey;
        privateKey.MakePublicKey(publicKey);
        result = publicKey.Validate(prng, 3);
        if(!result) {
                throw std::runtime_error("Unable to validate generated pubkey!");
        }
        
        // Export private, public key to hex strings
        CryptoPP::HexEncoder encoderPriv;
        encoderPriv.Attach(new CryptoPP::StringSink(privateKeyStr));
        privateKey.Save(encoderPriv);
        encoderPriv.MessageEnd();

        CryptoPP::HexEncoder encoderPub;
        encoderPub.Attach(new CryptoPP::StringSink(publicKeyStr));
        publicKey.Save(encoderPub);
        encoderPub.MessageEnd();
}


