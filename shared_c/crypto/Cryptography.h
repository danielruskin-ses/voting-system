#include "../Definitions.h"
#include "gmp.h"

#include <vector>

#define RSA_PRIVATE_KEY_SIZE 256

// TODO: handle failed realloc, malloc across codebase
// TODO: These lengths are really incorrect, fix them
#define RSA_PRIVATE_KEY_SIZE_DER 2048
#define RSA_PUBLIC_KEY_SIZE_DER 2048
#define RSA_PRIVATE_KEY_SIZE_B64 2048
#define RSA_PUBLIC_KEY_SIZE_B64 2048

#define RSA_SIGNATURE_SIZE RSA_PRIVATE_KEY_SIZE
#define RSA_ENC_SIZE RSA_PRIVATE_KEY_SIZE

#define AES_KEY_SIZE_BYTES 16

#define P_KEY_SIZE 2048
#define P_CIPHERTEXT_MAX_LEN 512

#define CRYPTO_ERROR -1

int generateKeypair(unsigned int keySize, BYTE_T* pubKey, unsigned int* pubKeyLen, BYTE_T* privKey, unsigned int* privKeyLen);

// This function ALLOCATES MEMORY to store its results.
int rsaSign(const BYTE_T* msg, unsigned int msgLen, const BYTE_T* privKey, unsigned int privKeyLen, BYTE_T** out, unsigned int* outLen);

bool rsaVerify(BYTE_T* msg, unsigned int msgLen, const BYTE_T* sig, unsigned int sigLen, const BYTE_T* pubKey, unsigned int pubKeyLen);

// This function ALLOCATES MEMORY to store its results.
int rsaEncrypt(const BYTE_T* msg, unsigned int msgLen, const BYTE_T* pubKey, unsigned int pubKeyLen, BYTE_T** encKeyOut, unsigned int* encKeyOutLen, BYTE_T** ivOut, unsigned int* ivOutLen, BYTE_T** encOut, unsigned int* encOutLen, unsigned int* encOutPadBytes);
int rsaDecrypt(const BYTE_T* privKey, unsigned int privKeyLen, const BYTE_T* encKey, unsigned int encKeyLen, const BYTE_T* iv, const BYTE_T* ct, unsigned int ctLen, unsigned int ctPadBytes, BYTE_T** msgOut, unsigned int* msgOutLen);

// output must be of length SHA256_DIGEST_SIZE
int sha256Hash(BYTE_T* output, BYTE_T* input, unsigned int inputLen);

// These functions assume that unsigned long ints are enc'd/dec'd.
// These functions ALLOCATE MEMORY to store their results.
// TODO A lot of stuff in this lib should really be const
std::vector<BYTE_T> exportMpz(const mpz_t& mpz);
void paillierKeygen(unsigned int bits, char** privHexP, char** privHexQ, char** pubHex);
void paillierEnc(char* plaintext, int plaintextLen, char* pubHex, void** ctext, char* custom_rand, int custom_rand_len);
void paillierDec(char* ctext, unsigned int ctextSize, char* privPHex, char* privQHex, char* pubHex, unsigned int plaintextLen, char** plaintext);

bool paillierGetRand(char* ctext, unsigned int ctextSize, char* privPHex, char* privQHex, char* pubHex, char** rand, long unsigned int* randSize);
void paillierSum(void** ctextOut, char** ctextsIn, int* ctextSizesIn, int numCtextIn, char* pubHex);

void randomGroupValue(const char* vtmfGroup, int vtmfGroupSize, unsigned int* outLen, char** out);
void elGamalShuffle(const char* vtmfGroup, int vtmfGroupLen, const char* vtmfKey, int vtmfKeyLen, std::vector<std::pair<mpz_t, mpz_t>>& original, std::vector<std::pair<mpz_t, mpz_t>>& out, std::vector<BYTE_T>& proofOut);
void elGamalEncrypt(const char* vtmfGroup, int vtmfGroupLen, const char* vtmfKey, int vtmfKeyLen, const char* msg, int msgLen, char** encA, unsigned int* encALen, char** encB, unsigned int* encBLen);
void elGamalDecrypt(const char* vtmfGroup, int vtmfGroupLen, const char* xHex, int xLen, const char* encA, int encALen, const char* encB, int encBLen, char** dec, unsigned int* decLen);
