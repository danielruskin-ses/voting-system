#include "../Definitions.h"

#define RSA_KEY_SIZE 256
#define RSA_KEY_SIZE_DER 2048 // TODO: fix this 
#define RSA_SIGNATURE_SIZE RSA_KEY_SIZE / 8
#define CRYPTO_ERROR -1

// pubKeyLen, privKeyLen should equal RSA_KEY_SIZE_DER
int createKeypair(unsigned int keySize, BYTE_T* pubKey, unsigned int* pubKeyLen, BYTE_T* privKey, unsigned int* privKeyLen);

// outLen should equal RSA_SIGNATURE_SIZE (and out should be of that size)
int rsaSign(BYTE_T* msg, unsigned int msgLen, BYTE_T* privKey, unsigned int privKeyLen, BYTE_T* out, unsigned int outLen);

bool rsaVerify(BYTE_T* msg, unsigned int msgLen, BYTE_T* sig, unsigned int sigLen, BYTE_T* pubKey, unsigned int pubKeyLen);
