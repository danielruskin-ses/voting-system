#include <string>
#include <exception>

class CryptoError : public std::exception {
public:
        CryptoError(const std::string& err) : _err(err) {
        }

        const char* what() const throw () {
                return _err.c_str();
        }

private:
        std::string _err;
};

void GenerateKeyPair(std::string& privateKey, std::string& publicKey);
