#include "Crypto.shared.h"

#include <cryptopp/eccrypto.h>
#include <cryptopp/osrng.h>
#include <cryptopp/oids.h>
#include <cryptopp/hex.h>

using namespace CryptoPP;

void GenerateKeyPair(std::string& privateKeyStr, std::string& publicKeyStr) {
        // Generate private key
        AutoSeededRandomPool prng;
        ECDSA<ECP, SHA1>::PrivateKey privateKey;
        privateKey.Initialize(prng, ASN1::secp160r1()); // TODO: use better curve
        bool result = privateKey.Validate(prng, 3);
        if(!result) {
                throw std::runtime_error("Unable to validate generated privkey!");
        }

        // Generate public key
        ECDSA<ECP, SHA1>::PublicKey publicKey;
        privateKey.MakePublicKey(publicKey);
        result = publicKey.Validate(prng, 3);
        if(!result) {
                throw std::runtime_error("Unable to validate generated pubkey!");
        }
        
        // Export private, public key to hex strings
        HexEncoder encoderPriv;
        encoderPriv.Attach(new StringSink(privateKeyStr));
        privateKey.Save(encoderPriv);
        encoderPriv.MessageEnd();

        HexEncoder encoderPub;
        encoderPub.Attach(new StringSink(publicKeyStr));
        publicKey.Save(encoderPub);
        encoderPub.MessageEnd();
}

std::string SignMessage(const std::string& message, const std::string& privateKeyStr) {
        // Load private key str into hex decoder 
        HexDecoder privateKeyDecoder;
        privateKeyDecoder.Put((byte*)&privateKeyStr[0], privateKeyStr.size());

        // Load private key from hex decoder, validate
        AutoSeededRandomPool prng;
        ECDSA<ECP, SHA1>::PrivateKey privateKey;
        privateKey.Load(privateKeyDecoder);
        bool result = privateKey.Validate(prng, 3);
        if(!result) {
                throw std::runtime_error("Unable to validate loaded privkey!");
        }

        // Initialize signer
        ECDSA<ECP, SHA1>::Signer signer(privateKey);

        // Determine maximum size, allocate a string with the maximum size
        size_t siglen = signer.MaxSignatureLength();
        std::string signature(siglen, 0x00);
        
        // Sign, and trim signature to actual size
        siglen = signer.SignMessage(prng, (const byte*)&message[0], message.size(), (byte*)&signature[0]);
        signature.resize(siglen);

        return signature;
}

bool VerifyMessage(const std::string& message, const std::string& signature, const std::string& publicKeyStr) {
        // Load public key str into hex decoder 
        HexDecoder publicKeyDecoder;
        publicKeyDecoder.Put((byte*)&publicKeyStr[0], publicKeyStr.size());

        // Load private key from hex decoder, validate
        AutoSeededRandomPool prng;
        ECDSA<ECP, SHA1>::PublicKey publicKey;
        publicKey.Load(publicKeyDecoder);
        bool result = publicKey.Validate(prng, 3);
        if(!result) {
                throw std::runtime_error("Unable to validate loaded pubkey!");
        }

        // Initialize verifier
        ECDSA<ECP, SHA1>::Verifier verifier(publicKey);

        return verifier.VerifyMessage((const byte*)&message[0], message.size(), (const byte*)&signature[0], signature.size() );
}
