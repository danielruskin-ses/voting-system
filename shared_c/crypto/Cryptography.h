#include "../Definitions.h"

#define RSA_PRIVATE_KEY_SIZE 256

// TODO: These lengths are really incorrect, fix them
#define RSA_PRIVATE_KEY_SIZE_DER 2048
#define RSA_PUBLIC_KEY_SIZE_DER 2048
#define RSA_PRIVATE_KEY_SIZE_B64 2048
#define RSA_PUBLIC_KEY_SIZE_B64 2048

#define RSA_SIGNATURE_SIZE RSA_KEY_SIZE / 8

#define CRYPTO_ERROR -1

int generateKeypair(unsigned int keySize, BYTE_T* pubKey, unsigned int* pubKeyLen, BYTE_T* privKey, unsigned int* privKeyLen);
int rsaSign(BYTE_T* msg, unsigned int msgLen, BYTE_T* privKey, unsigned int privKeyLen, BYTE_T* out, unsigned int outLen);
bool rsaVerify(BYTE_T* msg, unsigned int msgLen, BYTE_T* sig, unsigned int sigLen, BYTE_T* pubKey, unsigned int pubKeyLen);
