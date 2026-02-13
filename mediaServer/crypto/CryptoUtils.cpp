#include "CryptoUtils.h"
#include <openssl/conf.h>
#include <openssl/err.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

const std::string CryptoUtils::base64_chars_ = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

CryptoUtils::CryptoUtils() : initialized_(false) {
    // 初始化OpenSSL
    if (OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CONFIG, NULL) != 1) {
        std::cerr << "Failed to initialize OpenSSL" << std::endl;
        return;
    }
    
    // 初始化随机数生成器
    if (RAND_poll() != 1) {
        std::cerr << "Failed to seed random number generator" << std::endl;
        return;
    }
    
    initialized_ = true;
}

CryptoUtils::~CryptoUtils() {
    // OpenSSL的清理会由OPENSSL_cleanup()处理
    // 在程序结束时自动调用
}

CryptoUtils& CryptoUtils::getInstance() {
    static CryptoUtils instance;
    return instance;
}

std::string CryptoUtils::hashPassword(const std::string& password, const std::string& salt) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    
    std::string actualSalt = salt.empty() ? generateSalt() : salt;
    
    // 使用PBKDF2进行密码哈希
    auto derivedKey = pbkdf2(password, actualSalt, 100000, 32); // 100,000 iterations
    
    // 将盐和哈希值组合输出
    std::string result = actualSalt;
    result.append(reinterpret_cast<const char*>(derivedKey.data()), derivedKey.size());
    
    return base64Encode(result);
}

bool CryptoUtils::verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    
    try {
        std::vector<uint8_t> decoded = base64Decode(hash);
        
        if (decoded.size() < 32) { // 盐至少应该是32字节
            return false;
        }
        
        std::string actualSalt(reinterpret_cast<const char*>(decoded.data()), 32);
        std::vector<uint8_t> storedHash(decoded.begin() + 32, decoded.end());
        
        auto computedHash = pbkdf2(password, actualSalt, 100000, 32);
        
        return computedHash.size() == storedHash.size() &&
               CRYPTO_memcmp(computedHash.data(), storedHash.data(), computedHash.size()) == 0;
    } catch (const std::exception& e) {
        std::cerr << "Password verification error: " << e.what() << std::endl;
        return false;
    }
}

std::string CryptoUtils::generateSalt(size_t length) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    return generateRandomBytes(length);
}

std::vector<uint8_t> CryptoUtils::pbkdf2(const std::string& password, const std::string& salt, 
                                         int iterations, int keyLength) {
    std::vector<uint8_t> result(keyLength);
    
    // 解码Base64编码的salt
    std::vector<uint8_t> saltBytes = base64Decode(salt);
    
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                           saltBytes.data(), saltBytes.size(),
                           iterations, EVP_sha256(),
                           keyLength, result.data()) != 1) {
        throw std::runtime_error("PBKDF2 failed");
    }
    
    return result;
}

std::string CryptoUtils::aesEncrypt(const std::string& plaintext, const std::string& key) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    try {
        std::vector<uint8_t> iv(16);
        if (RAND_bytes(iv.data(), iv.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to generate IV");
        }
        
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, 
                              reinterpret_cast<const unsigned char*>(key.c_str()), 
                              iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }
        
        std::vector<uint8_t> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
        int len;
        int ciphertext_len = 0;
        
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                             reinterpret_cast<const unsigned char*>(plaintext.c_str()),
                             plaintext.length()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to encrypt data");
        }
        ciphertext_len += len;
        
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize encryption");
        }
        ciphertext_len += len;
        
        EVP_CIPHER_CTX_free(ctx);
        
        // 将IV和密文组合
        std::string result;
        result.append(reinterpret_cast<const char*>(iv.data()), iv.size());
        result.append(reinterpret_cast<const char*>(ciphertext.data()), ciphertext_len);
        
        return base64Encode(result);
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
}

std::string CryptoUtils::aesDecrypt(const std::string& ciphertext, const std::string& key) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    
    std::vector<uint8_t> decoded = base64Decode(ciphertext);
    
    if (decoded.size() < 16) { // 至少需要IV
        throw std::runtime_error("Invalid ciphertext format");
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create cipher context");
    }
    
    try {
        std::vector<uint8_t> iv(decoded.begin(), decoded.begin() + 16);
        std::vector<uint8_t> ciphertext_data(decoded.begin() + 16, decoded.end());
        
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                              reinterpret_cast<const unsigned char*>(key.c_str()),
                              iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }
        
        std::vector<uint8_t> plaintext(ciphertext_data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
        int len;
        int plaintext_len = 0;
        
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                             ciphertext_data.data(), ciphertext_data.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to decrypt data");
        }
        plaintext_len += len;
        
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize decryption");
        }
        plaintext_len += len;
        
        EVP_CIPHER_CTX_free(ctx);
        
        return std::string(reinterpret_cast<const char*>(plaintext.data()), plaintext_len);
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
}

std::string CryptoUtils::generateAESKey(size_t length) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    return generateRandomBytes(length);
}

std::string CryptoUtils::generateToken(size_t length) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    return generateRandomBytes(length);
}

std::string CryptoUtils::hmacSHA256(const std::string& data, const std::string& key) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    
    unsigned char hmac[EVP_MAX_MD_SIZE];
    unsigned int hmac_len;
    
    if (HMAC(EVP_sha256(), key.c_str(), key.length(),
              reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
              hmac, &hmac_len) == NULL) {
        throw std::runtime_error("HMAC calculation failed");
    }
    
    std::stringstream ss;
    for (unsigned int i = 0; i < hmac_len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hmac[i]);
    }
    
    return ss.str();
}

bool CryptoUtils::verifyHMAC(const std::string& data, const std::string& signature, const std::string& key) {
    try {
        std::string computedSignature = hmacSHA256(data, key);
        return CRYPTO_memcmp(computedSignature.c_str(), signature.c_str(), 
                            computedSignature.length()) == 0;
    } catch (...) {
        return false;
    }
}

std::string CryptoUtils::base64Encode(const std::vector<uint8_t>& data) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t in_len = data.size();
    
    while (in_len--) {
        char_array_3[i++] = data[(size_t)(data.size() - in_len - 1)];
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++) {
                ret += base64_chars_[char_array_4[i]];
            }
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
        
        for (j = 0; j < i + 1; j++) {
            ret += base64_chars_[char_array_4[j]];
        }
        
        while (i++ < 3) {
            ret += '=';
        }
    }
    
    return ret;
}

std::string CryptoUtils::base64Encode(const std::string& data) {
    return base64Encode(std::vector<uint8_t>(data.begin(), data.end()));
}

std::vector<uint8_t> CryptoUtils::base64Decode(const std::string& encoded) {
    return base64DecodeInternal(encoded);
}

bool CryptoUtils::isBase64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::vector<uint8_t> CryptoUtils::base64DecodeInternal(const std::string& encoded) {
    size_t in_len = encoded.size();
    int i = 0;
    int j = 0;
    int in = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<uint8_t> ret;
    
    while (in_len-- && (encoded[in] != '=') && isBase64(encoded[in])) {
        char_array_4[i++] = encoded[in]; in++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars_.find(char_array_4[i]);
            }
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (i = 0; i < 3; i++) {
                ret.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }
    
    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }
        
        for (j = 0; j < 4; j++) {
            char_array_4[j] = base64_chars_.find(char_array_4[j]);
        }
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        
        for (j = 0; j < i - 1; j++) {
            ret.push_back(char_array_3[j]);
        }
    }
    
    return ret;
}

std::string CryptoUtils::sha256(const std::string& data) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

std::string CryptoUtils::generateRandomBytes(size_t length) {
    if (!initialized_) {
        throw std::runtime_error("CryptoUtils not initialized");
    }
    
    std::vector<unsigned char> buffer(length);
    if (RAND_bytes(buffer.data(), buffer.size()) != 1) {
        throw std::runtime_error("Failed to generate random bytes");
    }
    
    // 转换为Base64编码，确保字符串安全
    return base64Encode(buffer);
}