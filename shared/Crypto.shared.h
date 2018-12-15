#pragma once 

#include "shared.grpc.pb.h"

#include <cryptopp/eccrypto.h>
#include <cryptopp/osrng.h>
#include <cryptopp/oids.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha3.h>

#include <google/protobuf/util/message_differencer.h>

void GenerateKeyPair(std::string& publicKeyStr, std::string& privateKeyStr);

template<typename T>
void HashMessage(const T& message, Hash* outputHash) {
        CryptoPP::SHA3_512 hash;

        // Generate serialized message
        std::string serialized;
        message.SerializeToString(&serialized);
        
        // Calculate digest
        byte digest[CryptoPP::SHA3_512::DIGESTSIZE];
        hash.CalculateDigest(digest, (const byte*) serialized.c_str(), serialized.size());
        
        // Convert digest to string
        std::string output;
        CryptoPP::HexEncoder encoder;
        encoder.Attach(new CryptoPP::StringSink(output));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();

        // Populate output hash
        outputHash->set_serializeddata(serialized);
        outputHash->set_hash(output);
}

template<typename T>
bool VerifyHash(const T& message, const Hash& hashIn) {
        // Generate hash of hash.serializeddata
        CryptoPP::SHA3_512 hash;
        byte digest[CryptoPP::SHA3_512::DIGESTSIZE];
        hash.CalculateDigest(digest, (const byte*) hashIn.serializeddata().c_str(), hashIn.serializeddata().size());
        std::string hashGen;
        CryptoPP::HexEncoder encoder;
        encoder.Attach(new CryptoPP::StringSink(hashGen));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();

        // Parse message from hash.serializeddata
        T messageFromHash;
        messageFromHash.ParseFromString(hashIn.serializeddata());

        // In order for a hash to be valid, two conditions must be met:
        // 1. hash.hash must be a correct hash of hash.serializedData
        // 2. hash.serializedData must represent the same message as message.
        return (
                hashIn.hash() == hashGen &&
                google::protobuf::util::MessageDifferencer::Equals(message, messageFromHash)
        );
}

template<typename T>
void SignMessage(const T& message, Signature* outputSig, const std::string privateKeyStr) {
        // Load private key str into hex decoder 
        CryptoPP::HexDecoder privateKeyDecoder;
        privateKeyDecoder.Put((byte*)&privateKeyStr[0], privateKeyStr.size());

        // Load private key from hex decoder, validate
        CryptoPP::AutoSeededRandomPool prng;
        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PrivateKey privateKey;
        privateKey.Load(privateKeyDecoder);
        bool result = privateKey.Validate(prng, 3);
        if(!result) {
                throw std::runtime_error("Unable to validate loaded privkey!");
        }

        // Initialize signer
        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::Signer signer(privateKey);

        // Determine maximum size, allocate a string with the maximum size
        size_t siglen = signer.MaxSignatureLength();
        byte decodedSig[siglen];

        // Generate serialized message
        std::string serialized;
        message.SerializeToString(&serialized);
        
        // Sign message => byte array
        siglen = signer.SignMessage(prng, (const byte*)&(serialized[0]), serialized.size(), decodedSig);

        // Convert byte array to hex
        CryptoPP::HexEncoder encoder;
        encoder.Put(decodedSig, siglen);
        encoder.MessageEnd();

        // Dump hex to string 
        std::string encodedSig;
        encodedSig.resize(encoder.MaxRetrievable());
        encoder.Get((byte*) &(encodedSig[0]), encodedSig.size());

        // Populate output sig
        outputSig->set_serializeddata(serialized);
        outputSig->set_signature(encodedSig);
}

template<typename T>
bool VerifyMessage(const T& message, const Signature& signature, const std::string& publicKeyStr) {
        // Load public key str into hex decoder 
        CryptoPP::HexDecoder publicKeyDecoder;
        publicKeyDecoder.Put((byte*)&publicKeyStr[0], publicKeyStr.size());
        publicKeyDecoder.MessageEnd();

        // Load private key from hex decoder, validate
        CryptoPP::AutoSeededRandomPool prng;
        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::PublicKey publicKey;
        publicKey.Load(publicKeyDecoder);
        bool result = publicKey.Validate(prng, 3);
        if(!result) {
                throw std::runtime_error("Unable to validate loaded pubkey!");
        }

        // Load signature into hex decoder
        CryptoPP::HexDecoder signatureDecoder;
        signatureDecoder.Put((byte*) signature.signature().data(), signature.signature().size());
        signatureDecoder.MessageEnd();
        
        // Get signtaure from hex decoder
        byte decodedSig[signatureDecoder.MaxRetrievable()];
        signatureDecoder.Get(decodedSig, sizeof(decodedSig));

        // Initialize verifier
        CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA1>::Verifier verifier(publicKey);

        // Parse message from serialized data
        T messageFromSig;
        messageFromSig.ParseFromString(signature.serializeddata());

        // In order for a signature to be valid, two conditions must be met:
        // 1. signature.signature must be a valid signature of signature.serializedData
        // 2. signature.serializedData must represent the same message as message.
        return (
                verifier.VerifyMessage((const byte*) &(signature.serializeddata()[0]), signature.serializeddata().size(), decodedSig, sizeof(decodedSig)) &&
                google::protobuf::util::MessageDifferencer::Equals(message, messageFromSig)
        );
}
