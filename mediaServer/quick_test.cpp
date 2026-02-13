#include <iostream>
#include "crypto/CryptoUtils.h"

int main() {
    try {
        // 初始化加密工具
        CryptoUtils& crypto = CryptoUtils::getInstance();
        
        // 生成salt
        std::string salt = crypto.generateSalt(32);
        std::cout << "Generated salt: " << salt << std::endl;
        std::cout << "Salt length: " << salt.length() << std::endl;
        
        // 检查salt是否只包含安全字符
        bool safe = true;
        for (char c : salt) {
            if (c < 32 || c > 126) {
                safe = false;
                break;
            }
        }
        std::cout << "Salt is safe: " << (safe ? "YES" : "NO") << std::endl;
        
        // 测试密码哈希
        std::string password = "123456";
        std::string hash = crypto.hashPassword(password, salt);
        std::cout << "Password hash: " << hash << std::endl;
        std::cout << "Hash length: " << hash.length() << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}