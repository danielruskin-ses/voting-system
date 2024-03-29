#include "Cryptography.h"

#define WOLFSSL_KEY_GEN
#include "wolfssl/options.h"
#include "wolfssl/wolfcrypt/coding.h"
#include "wolfssl/wolfcrypt/rsa.h"
#include "wolfssl/wolfcrypt/sha256.h"

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

void paillierKeygen(unsigned int bits, char** privHexP, char** privHexQ, char** pubHex) {
        paillier_pubkey_t* pub;
        paillier_prvkey_t* priv;

        paillier_keygen(bits, &pub, &priv, paillier_get_rand_devurandom);
        *pubHex = paillier_pubkey_to_hex(pub);
        paillier_prvkey_to_hex(privHexP, privHexQ, priv);

        paillier_freepubkey(pub);
        paillier_freeprvkey(priv);
}

void paillierEnc(unsigned long int ptext, char* pubHex, void** ctext, char* custom_rand, int custom_rand_len) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        paillier_plaintext_t* pt = paillier_plaintext_from_bytes(&ptext, sizeof(unsigned long int));

        paillier_ciphertext_t* ct = paillier_enc(NULL, pub, pt, paillier_get_rand_devurandom, custom_rand, custom_rand_len);

        // TODO: this method is really dangerous if P_CIPHERTEXT_MAX_LEN is not long enough
        *ctext = paillier_ciphertext_to_bytes(P_CIPHERTEXT_MAX_LEN, ct);

        paillier_freepubkey(pub);
        paillier_freeplaintext(pt);
        paillier_freeciphertext(ct);
}

void paillierDec(char* ctext, unsigned int ctextSize, char* privPHex, char* privQHex, char* pubHex, unsigned long int* ptext) {
        paillier_pubkey_t* pub = paillier_pubkey_from_hex(pubHex);
        paillier_prvkey_t* priv = paillier_prvkey_from_hex(privPHex, privQHex, pub);
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
