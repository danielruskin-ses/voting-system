#include "../Definitions.h"

#define RSA_PRIVATE_KEY_SIZE 256

// TODO: These lengths are really incorrect, fix them
#define RSA_PRIVATE_KEY_SIZE_DER 2048
#define RSA_PUBLIC_KEY_SIZE_DER 2048
#define RSA_PRIVATE_KEY_SIZE_B64 2048
#define RSA_PUBLIC_KEY_SIZE_B64 2048

#define RSA_SIGNATURE_SIZE RSA_PRIVATE_KEY_SIZE

#define P_KEY_SIZE 2048
#define P_CIPHERTEXT_MAX_LEN 512

#define CRYPTO_ERROR -1

int generateKeypair(unsigned int keySize, BYTE_T* pubKey, unsigned int* pubKeyLen, BYTE_T* privKey, unsigned int* privKeyLen);
int rsaSign(const BYTE_T* msg, unsigned int msgLen, const BYTE_T* privKey, unsigned int privKeyLen, BYTE_T* out, unsigned int outLen);
bool rsaVerify(BYTE_T* msg, unsigned int msgLen, const BYTE_T* sig, unsigned int sigLen, const BYTE_T* pubKey, unsigned int pubKeyLen);

// These functions assume that unsigned long ints are enc'd/dec'd.
// These functions ALLOCATE MEMORY to store their results.
// TODO A lot of stuff in this lib should really be const
void paillierKeygen(unsigned int bits, char** privHex, char** pubHex);
void paillierEnc(unsigned long int ptext, char* pubHex, void** ctext);

void paillierDec(char* ctext, unsigned int ctextSize, char* privHex, char* pubHex, unsigned long int* ptext);
bool paillierGetRand(char* ctext, unsigned int ctextSize, char* privHex, char* pubHex, char** rand); // TODO: Make sure it is ok for rand to be revealed
void paillierSum(void** ctextOut, char** ctextsIn, int* ctextSizesIn, int numCtextIn, char* pubHex);
