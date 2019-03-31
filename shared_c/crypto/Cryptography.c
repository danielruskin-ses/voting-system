#include "Cryptography.h"

#define WOLFSSL_KEY_GEN
#include "wolfssl/options.h"
#include "wolfssl/wolfcrypt/coding.h"
#include "wolfssl/wolfcrypt/rsa.h"

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

int rsaSign(BYTE_T* msg, unsigned int msgLen, BYTE_T* privKey, unsigned int privKeyLen, BYTE_T* out, unsigned int outLen) {
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

        wc_RsaSSL_Sign(msg, msgLen, out, outLen, &key, &rng);
        wc_FreeRsaKey(&key);
        return keyDecodeRes; // Len of signature
}

bool rsaVerify(BYTE_T* msg, unsigned int msgLen, BYTE_T* sig, unsigned int sigLen, BYTE_T* pubKey, unsigned int pubKeyLen) {
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        unsigned int idx = 0;
        int keyDecodeRes = wc_RsaPublicKeyDecode(pubKey, &idx, &key, pubKeyLen);
        if(keyDecodeRes != 0) {
                wc_FreeRsaKey(&key);
                return CRYPTO_ERROR;
        }

        int res = wc_RsaSSL_Verify(sig, sigLen, msg, sizeof(msgLen), &key);
        wc_FreeRsaKey(&key);
        if(res < 0) {
                return CRYPTO_ERROR;
        }

        return 0;
}
