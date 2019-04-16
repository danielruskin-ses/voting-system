#include "Cryptography.h"

#define WOLFSSL_KEY_GEN
#include "wolfssl/options.h"
#include "wolfssl/wolfcrypt/coding.h"
#include "wolfssl/wolfcrypt/rsa.h"
#include "wolfssl/wolfcrypt/sha256.h"

#include "gmp.h" // must include before paillier
extern "C" {
        // must wrap in extern C b/c this is a C lib
        #include "paillier.h"
}

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

int rsaSign(const BYTE_T* msg, unsigned int msgLen, const BYTE_T* privKey, unsigned int privKeyLen, BYTE_T* out, unsigned int outLen) {
        // First, hash message
        Sha256 sha256;
        unsigned char hash[SHA256_DIGEST_SIZE];
        wc_InitSha256(&sha256);
        wc_Sha256Update(&sha256, msg, msgLen);
        wc_Sha256Final(&sha256, hash);

        // Then, sign hash 
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        unsigned int idx = 0;
        int keyDecodeRes = wc_RsaPrivateKeyDecode(privKey, &idx, &key, privKeyLen);
        if(keyDecodeRes != 0) {
                wc_FreeRsaKey(&key);
                return CRYPTO_ERROR;
        }

        // TODO: use one rng?
        // TODO: use harden options?
        RNG rng;
        wc_InitRng(&rng);

        int signRes = wc_RsaSSL_Sign(hash, SHA256_DIGEST_SIZE, out, outLen, &key, &rng);
        if(signRes < 0) {
                return CRYPTO_ERROR;
        }
        wc_FreeRsaKey(&key);
        return signRes; // Len of signature
}

bool rsaVerify(BYTE_T* msg, unsigned int msgLen, const BYTE_T* sig, unsigned int sigLen, const BYTE_T* pubKey, unsigned int pubKeyLen) {
        // First, hash message
        Sha256 sha256;
        unsigned char hash[SHA256_DIGEST_SIZE];
        wc_InitSha256(&sha256);
        wc_Sha256Update(&sha256, msg, msgLen);
        wc_Sha256Final(&sha256, hash);

        // Then, verify hash
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        unsigned int idx = 0;
        int keyDecodeRes = wc_RsaPublicKeyDecode(pubKey, &idx, &key, pubKeyLen);
        if(keyDecodeRes != 0) {
                wc_FreeRsaKey(&key);
                return false;
        }

        int res = wc_RsaSSL_Verify(sig, sigLen, hash, SHA256_DIGEST_SIZE, &key);
        wc_FreeRsaKey(&key);
        if(res < 0) {
                return false;
        }

        return true;
}

void paillierKeygen(unsigned int bits, char** privHex, char** pubHex) {
        paillier_pubkey_t* pub;
        paillier_prvkey_t* priv;

        paillier_keygen(bits, &pub, &priv, paillier_get_rand_devurandom);
        *pubHex = paillier_pubkey_to_hex(pub);
        *privHex = paillier_prvkey_to_hex(priv);

        paillier_freepubkey(pub);
        paillier_freeprvkey(priv);
}

void paillierEnc(unsigned long int ptext, char* pubHex, void** ctext) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        paillier_plaintext_t* pt = paillier_plaintext_from_bytes(&ptext, sizeof(unsigned long int));

        paillier_ciphertext_t* ct = paillier_enc(NULL, pub, pt, paillier_get_rand_devurandom);

        // TODO: this method is really dangerous if P_CIPHERTEXT_MAX_LEN is not long enough
        *ctext = paillier_ciphertext_to_bytes(P_CIPHERTEXT_MAX_LEN, ct);

        paillier_freepubkey(pub);
        paillier_freeplaintext(pt);
        paillier_freeciphertext(ct);
}

void paillierDec(char* ctext, unsigned int ctextSize, char* privHex, char* pubHex, unsigned long int* ptext) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        paillier_prvkey_t* priv = paillier_prvkey_from_hex(privHex, pub);
        paillier_ciphertext_t* ct = paillier_ciphertext_from_bytes((void*) ctext, ctextSize);
        paillier_plaintext_t* pt = paillier_dec(NULL, pub, priv, ct);

        void* bytes = paillier_plaintext_to_bytes(sizeof(unsigned long int), pt);
        *ptext = *((unsigned long int*) bytes);
        free(bytes);
        
        paillier_freepubkey(pub);
        paillier_freeprvkey(priv);
        paillier_freeciphertext(ct);
        paillier_freeplaintext(pt);
}

void paillierGetRand(char* ctext, unsigned int ctextSize, char* privHex, char* pubHex, int* rand) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        paillier_prvkey_t* priv = paillier_prvkey_from_hex(privHex, pub);
        paillier_ciphertext_t* ct = paillier_ciphertext_from_bytes((void*) ctext, ctextSize);
        paillier_plaintext_t* pt = paillier_dec(NULL, pub, priv, ct);
        
        // TODO
        
        paillier_freepubkey(pub);
        paillier_freeprvkey(priv);
        paillier_freeciphertext(ct);
        paillier_freeplaintext(pt);
}

void paillierSum(char** ctextOut, char** ctextsIn, int* ctextSizesIn, int numCtextIn, char* pubHex) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        
        paillier_ciphertext_t* sum = paillier_create_enc_zero();
        for(int i = 0; i < numCtextIn; i++) {
                paillier_ciphertext_t* ct = paillier_ciphertext_from_bytes((void*) ctextsIn[i], ctextSizesIn[i]);
                paillier_mul(pub, sum, sum, ct);
                paillier_free_ciphertext(ct);
        }

        paillier_freepubkey(pub);

        *ctextOut = paillier_ciphertext_to_bytes(P_CIPHERTEXT_MAX_LEN, sum);

        paillier_free_ciphertext(sum);
}
