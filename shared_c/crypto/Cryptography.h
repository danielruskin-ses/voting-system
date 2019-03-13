#define RSA_KEY_SIZE 2048
#define RSA_SIGNATURE_SIZE RSA_KEY_SIZE / 8
#define CRYPTO_ERROR -1

// outLen should equal RSA_SIGNATURE_SIZE (and out should be of that size)
int rsaSign(char* msg, int msgLen, char* privKey, int privKeyLen, char* out, char* outLen);

bool rsaVerify(char* msg, int msgLen, char* pubKey, int pubKeyLen);
