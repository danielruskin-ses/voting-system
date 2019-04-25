#include "Cryptography.h"

#define WOLFSSL_KEY_GEN
#include "wolfssl/options.h"
#include "wolfssl/wolfcrypt/coding.h"
#include "wolfssl/wolfcrypt/rsa.h"
#include "wolfssl/wolfcrypt/sha256.h"
#include "wolfssl/wolfcrypt/aes.h"
#include "wolfssl/wolfcrypt/signature.h"

#include "gmp.h" // must include before paillier
#include "paillier_lib/paillier.h"

// TODO: update this library to make required lengths more clear - ex are pubkeys always the same length as privkeys?
// Do the same wherever base64 encoding is used.
// TODO: should ints here be unsigned?
int generateKeypair(unsigned int keySize, BYTE_T* pubKey, unsigned int* pubKeyLen, BYTE_T* privKey, unsigned int* privKeyLen) {
        static long e = 65537;

        // Generate key
        RsaKey priv;
        wc_InitRsaKey(&priv, NULL);
        RNG rng;
        wc_InitRng(&rng);
        int ret = wc_MakeRsaKey(&priv, keySize * 8, e, &rng);
        if(ret != 0) {
                wc_FreeRsaKey(&priv);
                return CRYPTO_ERROR;
        }

        // Encode privkey
        ret = wc_RsaKeyToDer(&priv, privKey, *privKeyLen);
        if(ret <= 0) {
                wc_FreeRsaKey(&priv);
                return CRYPTO_ERROR;
        } else {
                *privKeyLen = ret;
        }

        // Encode pubkey 
        ret = wc_RsaKeyToPublicDer(&priv, pubKey, *pubKeyLen);
        if(ret <= 0) {
                wc_FreeRsaKey(&priv);
                return CRYPTO_ERROR;
        } else {
                *pubKeyLen = ret;
        }

        wc_FreeRsaKey(&priv);
        return 0;
}

int sha256Hash(BYTE_T* output, BYTE_T* input, unsigned int inputLen) {
        Sha256 sha256;
        int res = wc_InitSha256(&sha256);
        if(res != 0) {
                return CRYPTO_ERROR;
        }

        res = wc_Sha256Update(&sha256, input, inputLen);
        if(res != 0) {
                return CRYPTO_ERROR;
        }

        res = wc_Sha256Final(&sha256, output);
        if(res != 0) {
                return CRYPTO_ERROR;
        }

        return 0;
}

int rsaSign(const BYTE_T* msg, unsigned int msgLen, const BYTE_T* privKey, unsigned int privKeyLen, BYTE_T** out, unsigned int* outLen) {
        // Decode key
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        unsigned int idx = 0;
        int keyDecodeRes = wc_RsaPrivateKeyDecode(privKey, &idx, &key, privKeyLen);
        if(keyDecodeRes != 0) {
                wc_FreeRsaKey(&key);
                return CRYPTO_ERROR;
        }

        // TODO: use harden options?
        RNG rng;
        wc_InitRng(&rng);

        // Alloc memory for signaure
        *outLen = wc_SignatureGetSize(WC_SIGNATURE_TYPE_RSA, &key, sizeof(key));
        *out = (BYTE_T*) malloc(*outLen);

        // Generate signature
        int signRes = wc_SignatureGenerate(
                WC_HASH_TYPE_SHA256,
                WC_SIGNATURE_TYPE_RSA,
                msg,
                msgLen,
                *out,
                outLen,
                &key,
                sizeof(key),
                &rng);
        wc_FreeRsaKey(&key);
        if(signRes != 0) {
                free(*out);
                return CRYPTO_ERROR;
        }
        return 0; 
}

bool rsaVerify(BYTE_T* msg, unsigned int msgLen, const BYTE_T* sig, unsigned int sigLen, const BYTE_T* pubKey, unsigned int pubKeyLen) {
        // Decode key
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        unsigned int idx = 0;
        int keyDecodeRes = wc_RsaPublicKeyDecode(pubKey, &idx, &key, pubKeyLen);
        if(keyDecodeRes != 0) {
                wc_FreeRsaKey(&key);
                return false;
        }

        // Verify signature
        int verifyRes = wc_SignatureVerify(
                WC_HASH_TYPE_SHA256,
                WC_SIGNATURE_TYPE_RSA,
                msg,
                msgLen,
                sig,
                sigLen,
                &key,
                sizeof(key));
        wc_FreeRsaKey(&key);
        return (verifyRes == 0);
}

int rsaEncrypt(const BYTE_T* msg, unsigned int msgLen, const BYTE_T* pubKey, unsigned int pubKeyLen, BYTE_T** encKeyOut, unsigned int* encKeyOutLen, BYTE_T** ivOut, unsigned int* ivOutLen, BYTE_T** encOut, unsigned int* encOutLen, unsigned int* encOutPadBytes) {
        // Generate AES Key bytes
        RNG rng;
        wc_InitRng(&rng);
        BYTE_T aesKey[AES_KEY_SIZE_BYTES];
        wc_RNG_GenerateBlock(&rng, aesKey, AES_KEY_SIZE_BYTES);
        
        // Decode RSA pubkey
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        unsigned int idx = 0;
        int keyDecodeRes = wc_RsaPublicKeyDecode(pubKey, &idx, &key, pubKeyLen);
        if(keyDecodeRes != 0) {
                wc_FreeRsaKey(&key);
                return CRYPTO_ERROR;
        }

        // Encrypt AES Key using RSA pub key
        *encKeyOut = (BYTE_T*) malloc(RSA_ENC_SIZE);
        *encKeyOutLen = RSA_ENC_SIZE;
        int res = wc_RsaPublicEncrypt(
                aesKey,
                AES_KEY_SIZE_BYTES,
                *encKeyOut,
                RSA_ENC_SIZE,
                &key,
                &rng
        );
        if(res != RSA_ENC_SIZE) {
                free(*encKeyOut);
                return CRYPTO_ERROR;
        }

        // Generate AES IV
        *ivOut = (BYTE_T*) malloc(AES_KEY_SIZE_BYTES);
        *ivOutLen = AES_KEY_SIZE_BYTES;
        wc_RNG_GenerateBlock(&rng, *ivOut, AES_KEY_SIZE_BYTES);

        // Initialize AES object 
        Aes aesEnc;
        res = wc_AesSetKey(&aesEnc, aesKey, AES_KEY_SIZE_BYTES, *ivOut, AES_ENCRYPTION);
        if(res != 0) {
                free(*encKeyOut);
                free(*ivOut);
                return CRYPTO_ERROR;
        }

        // Pad data
        int length = msgLen;
        int paddingLen = 0;
        while(length % AES_KEY_SIZE_BYTES != 0) {
                length++;
                paddingLen++;
        }
        BYTE_T msgNew[length];
        memcpy(msgNew, msg, msgLen);
        for(int i = msgLen; i < length; i++) {
                msgNew[i] = paddingLen;
        }

        // Encrypt data
        *encOutPadBytes = paddingLen;
        *encOut = (BYTE_T*) malloc(length);
        *encOutLen = length;
        res = wc_AesCbcEncrypt(&aesEnc, *encOut, msgNew, length);

        if(res != 0) {
                free(*encKeyOut);
                free(*ivOut);
                free(*encOut);
                return CRYPTO_ERROR;
        }

        return 0;
}

int rsaDecrypt(const BYTE_T* privKey, unsigned int privKeyLen, const BYTE_T* encKey, unsigned int encKeyLen, const BYTE_T* iv, const BYTE_T* ct, unsigned int ctLen, unsigned int ctPadBytes, BYTE_T** msgOut, unsigned int* msgOutLen) {
        // Decode RSA privkey
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        unsigned int idx = 0;
        int keyDecodeRes = wc_RsaPrivateKeyDecode(privKey, &idx, &key, privKeyLen);
        if(keyDecodeRes != 0) {
                wc_FreeRsaKey(&key);
                return CRYPTO_ERROR;
        }

        // TODO: this is needed for some reason
        RNG rng;
        wc_InitRng(&rng);
        wc_RsaSetRNG(&key, &rng);
        
        // Decrypt AES privkey
        BYTE_T aesKeyPt[AES_KEY_SIZE_BYTES];
        int res = wc_RsaPrivateDecrypt(
                encKey,
                encKeyLen,
                aesKeyPt,
                AES_KEY_SIZE_BYTES,
                &key);
        if(res != AES_KEY_SIZE_BYTES) {
                return CRYPTO_ERROR;
        }
        Aes aesDec;
        res = wc_AesSetKey(&aesDec, aesKeyPt, AES_KEY_SIZE_BYTES, iv, AES_DECRYPTION);
        if(res != 0) {
                return CRYPTO_ERROR;
        }

        // Decrypt
        *msgOut = (BYTE_T*) malloc(ctLen);
        res = wc_AesCbcDecrypt(&aesDec, *msgOut, ct, ctLen);
        if(res != 0) {
                free(*msgOut);
                return CRYPTO_ERROR;
        }

        // Undo padding
        *msgOut = (BYTE_T*) realloc(*msgOut, ctLen - ctPadBytes);
        *msgOutLen = ctLen - ctPadBytes;

        return 0;
}

void paillierKeygen(unsigned int bits, char** privHexP, char** privHexQ, char** pubHex) {
        paillier_pubkey_t* pub;
        paillier_prvkey_t* priv;

        paillier_keygen(bits, &pub, &priv, paillier_get_rand_devurandom);
        *pubHex = paillier_pubkey_to_hex(pub);
        paillier_prvkey_to_hex(privHexP, privHexQ, priv);

        paillier_freepubkey(pub);
        paillier_freeprvkey(priv);
}

void paillierEnc(char* plaintext, int plaintextLen, char* pubHex, void** ctext, char* custom_rand, int custom_rand_len) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        paillier_plaintext_t* pt = paillier_plaintext_from_bytes(plaintext, plaintextLen);

        paillier_ciphertext_t* ct = paillier_enc(NULL, pub, pt, paillier_get_rand_devurandom, custom_rand, custom_rand_len);

        // TODO: this method is really dangerous if P_CIPHERTEXT_MAX_LEN is not long enough
        *ctext = paillier_ciphertext_to_bytes(P_CIPHERTEXT_MAX_LEN, ct);

        paillier_freepubkey(pub);
        paillier_freeplaintext(pt);
        paillier_freeciphertext(ct);
}

void paillierDec(char* ctext, unsigned int ctextSize, char* privPHex, char* privQHex, char* pubHex, unsigned int plaintextLen, char** plaintext) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        paillier_prvkey_t* priv = paillier_prvkey_from_hex(privPHex, privQHex, pub);
        paillier_ciphertext_t* ct = paillier_ciphertext_from_bytes((void*) ctext, ctextSize);
        paillier_plaintext_t* pt = paillier_dec(NULL, pub, priv, ct);

        *plaintext = (char*) paillier_plaintext_to_bytes(plaintextLen, pt);
        
        paillier_freepubkey(pub);
        paillier_freeprvkey(priv);
        paillier_freeciphertext(ct);
        paillier_freeplaintext(pt);
}

bool paillierGetRand(char* ctext, unsigned int ctextSize, char* privPHex, char* privQHex, char* pubHex, char** randVal, long unsigned int* randSize) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        paillier_prvkey_t* priv = paillier_prvkey_from_hex(privPHex, privQHex, pub);
        paillier_ciphertext_t* ct = paillier_ciphertext_from_bytes((void*) ctext, ctextSize);
        paillier_plaintext_t* pt = paillier_dec(NULL, pub, priv, ct);

        // Init temp var
        mpz_t tempA;
        mpz_t tempB;
        mpz_t tempC;
        mpz_init(tempA);
        mpz_init(tempB);
        mpz_init(tempC);
        
        // tempA = 1 * P*n
        mpz_mul(tempA, pt->m, pub->n);
        mpz_ui_sub(tempA, 1, tempA);

        // Calculate C' = C * (1 - P*N) mod N^2 enc of 0, with same randomness as C
        // tempA = C * (1 - P*N) mod N^2
        mpz_mul(tempA, ct->c, tempA);
        mpz_mod(tempA, tempA, pub->n_squared);

        // Calculate tempB = tot(N) = (p-1)(q-1)
        mpz_sub_ui(tempC, priv->p, 1);
        mpz_sub_ui(tempB, priv->q, 1);
        mpz_mul(tempB, tempB, tempC);

        // tempB = N^-1 mod tot(n)
        int res = mpz_invert(tempB, pub->n, tempB);
        if(res == 0) {
                return false;
        }

        // tempA = r = tempA^tempB mod N
        mpz_powm(tempA, tempA, tempB, pub->n);

        *randVal = (char*) mpz_export(0, randSize, 1, 1, 0, 0, tempA);
        
        mpz_clear(tempA);
        mpz_clear(tempB);
        mpz_clear(tempC);
        paillier_freepubkey(pub);
        paillier_freeprvkey(priv);
        paillier_freeciphertext(ct);
        paillier_freeplaintext(pt);

        return true;
}

void paillierSum(void** ctextOut, char** ctextsIn, int* ctextSizesIn, int numCtextIn, char* pubHex) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        
        paillier_ciphertext_t* sum = paillier_create_enc_zero();
        for(int i = 0; i < numCtextIn; i++) {
                paillier_ciphertext_t* ct = paillier_ciphertext_from_bytes((void*) ctextsIn[i], ctextSizesIn[i]);
                paillier_mul(pub, sum, sum, ct);
                paillier_freeciphertext(ct);
        }

        paillier_freepubkey(pub);

        *ctextOut = paillier_ciphertext_to_bytes(P_CIPHERTEXT_MAX_LEN, sum);

        paillier_freeciphertext(sum);
}
