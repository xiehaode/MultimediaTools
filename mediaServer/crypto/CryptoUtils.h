#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <string>
#include <vector>
#include <memory>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/hmac.h>

class CryptoUtils {
public:
    static CryptoUtils& getInstance();
    
    // 析构函数
    ~CryptoUtils();
    
    // 禁用拷贝构造和赋值
    CryptoUtils(const CryptoUtils&) = delete;
    CryptoUtils& operator=(const CryptoUtils&) = delete;
    
    // 密码哈希（使用PBKDF2）
    std::string hashPassword(const std::string& password, const std::string& salt = "");
    bool verifyPassword(const std::string& password, const std::string& hash, const std::string& salt);
    
    // 生成随机盐
    std::string generateSalt(size_t length = 32);
    
    // AES加密/解密
    std::string aesEncrypt(const std::string& plaintext, const std::string& key);
    std::string aesDecrypt(const std::string& ciphertext, const std::string& key);
    
    // 生成AES密钥
    std::string generateAESKey(size_t length = 32);
    
    // 生成随机token
    std::string generateToken(size_t length = 64);
    
    // HMAC签名
    std::string hmacSHA256(const std::string& data, const std::string& key);
    bool verifyHMAC(const std::string& data, const std::string& signature, const std::string& key);
    
    // Base64编码/解码
    std::string base64Encode(const std::vector<uint8_t>& data);
    std::string base64Encode(const std::string& data);
    std::vector<uint8_t> base64Decode(const std::string& encoded);
    
    // SHA256哈希
    std::string sha256(const std::string& data);
    
    // 初始化检查
    bool isInitialized() const { return initialized_; }
    
private:
    CryptoUtils();
    
    bool initialized_;
    
    // 内部辅助函数
    std::vector<uint8_t> pbkdf2(const std::string& password, const std::string& salt, 
                                int iterations, int keyLength);
    std::string generateRandomBytes(size_t length);
    
    // Base64编解码辅助函数
    static const std::string base64_chars_;
    static bool isBase64(unsigned char c);
    std::vector<uint8_t> base64DecodeInternal(const std::string& encoded);
};

#endif // CRYPTOUTILS_H