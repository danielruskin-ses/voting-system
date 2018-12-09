#include <string>
#include <exception>

void GenerateKeyPair(std::string& publicKeyStr, std::string& privateKeyStr);
std::string SignMessage(const std::string& message, const std::string& publicKeyStr);
bool VerifyMessage(const std::string& message, const std::string& signature, const std::string& publicKeyStr);
