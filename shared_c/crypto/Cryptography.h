#include "../Definitions.h"

#define RSA_PRIVATE_KEY_SIZE 256

// TODO: These lengths are really incorrect, fix them
#define RSA_PRIVATE_KEY_SIZE_DER 2048
#define RSA_PUBLIC_KEY_SIZE_DER 2048
#define RSA_PRIVATE_KEY_SIZE_B64 2048
#define RSA_PUBLIC_KEY_SIZE_B64 2048

#define RSA_SIGNATURE_SIZE RSA_KEY_SIZE / 8

#define P_CIPHERTEXT_MAX_LEN 100

#define CRYPTO_ERROR -1

int generateKeypair(unsigned int keySize, BYTE_T* pubKey, unsigned int* pubKeyLen, BYTE_T* privKey, unsigned int* privKeyLen);
int rsaSign(BYTE_T* msg, unsigned int msgLen, BYTE_T* privKey, unsigned int privKeyLen, BYTE_T* out, unsigned int outLen);
bool rsaVerify(BYTE_T* msg, unsigned int msgLen, BYTE_T* sig, unsigned int sigLen, BYTE_T* pubKey, unsigned int pubKeyLen);

// These functions assume that unsigned long ints are enc'd/dec'd.
// These functions ALLOCATE MEMORY to store their results.
void paillierKeygen(unsigned int bits, char** privHex, char** pubHex);
void paillierEnc(unsigned long int ptext, char* pubHex, char** ctext);

// This function also assumes that ctext is of len P_CIPHERTEXT_MAX_LEN.
void paillierDec(char* ctext, char* privHex, char* pubHex, unsigned long int* ptext);
