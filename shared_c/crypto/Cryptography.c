#include <wolfssl/wolfcrypt/rsa>

#include "Cryptography.h"

int rsaSign(char* msg, int msgLen, char* privKey, int privKeyLen, char* out, char* outLen) {
        RsaKey key;
        int keyDecodeRes = wc_RsaPrivateKeyDecode(privKey, 0, &key, privKeyLen);
        if(keyDecodeRes != 0) {
                return CRYPTO_ERROR;
        }

        // TODO: use one rng?
        RNG rng;
        wc_InitRng(&rng);

        wc_RsaSSL_Sign(msg, msgLen, out, outLen, &key, rng);
        return 0;
}

bool rsaVerify(char* msg, int msgLen, char* pubKey, int pubKeyLen) {
        RsaKey key;
        int keyDecodeRes = wc_RsaPublicKeyDecode(pubKey, 0, &key, pubKeyLen);
        if(keyDecodeRes != 0) {
                return CRYPTO_ERROR;
        }

        int res = wc_RsaSSL_Verify(out, ret, plain, sizeof(plain), &key)
        if(res < 0) {
                return CRYPTO_ERROR;
        }

        return 0;
}
