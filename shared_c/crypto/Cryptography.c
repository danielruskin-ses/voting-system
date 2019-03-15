#include "wolfssl/wolfcrypt/rsa.h"

#include "Cryptography.h"

int rsaSign(BYTE_T* msg, int msgLen, BYTE_T* privKey, int privKeyLen, BYTE_T* out, int outLen) {
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        int keyDecodeRes = wc_RsaPrivateKeyDecode(privKey, 0, &key, privKeyLen);
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

bool rsaVerify(BYTE_T* msg, int msgLen, BYTE_T* sig, int sigLen, BYTE_T* pubKey, int pubKeyLen) {
        RsaKey key;
        wc_InitRsaKey(&key, NULL);
        int keyDecodeRes = wc_RsaPublicKeyDecode(pubKey, 0, &key, pubKeyLen);
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
