#include "../Definitions.h"

#define RSA_KEY_SIZE 2048
#define RSA_SIGNATURE_SIZE RSA_KEY_SIZE / 8
#define CRYPTO_ERROR -1

// outLen should equal RSA_SIGNATURE_SIZE (and out should be of that size)
int rsaSign(BYTE_T* msg, int msgLen, BYTE_T* privKey, int privKeyLen, BYTE_T* out, int outLen);

bool rsaVerify(BYTE_T* msg, int msgLen, BYTE_T* sig, int sigLen, BYTE_T* pubKey, int pubKeyLen);
