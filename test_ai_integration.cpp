#include <iostream>
#include <string>

// 包含curlAli API头文件
extern "C" {
#include "curlAli/curlAli.h"
}

int main() {
    std::cout << "测试AI功能集成..." << std::endl;
    
    // 测试API密钥（需要用户设置）
    const char* apiKey = "your-api-key-here"; // 用户需要替换为实际的API密钥
    
    std::cout << "1. 创建AI助手实例..." << std::endl;
    void* aiChat = DashScopeChat_Create(apiKey);
    
    if (!aiChat) {
        std::cout << "错误：无法创建AI助手实例，请检查API密钥是否正确。" << std::endl;
        return 1;
    }
    
    std::cout << "2. 发送测试消息..." << std::endl;
    const char* response = DashScopeChat_Chat(aiChat, "请生成一个ffmpeg命令来转换视频格式");
    
    if (response) {
        std::cout << "AI回复: " << response << std::endl;
    } else {
        std::cout << "错误：AI无响应，请检查网络连接和API配置。" << std::endl;
    }
    
    std::cout << "3. 清理资源..." << std::endl;
    DashScopeChat_Destroy(aiChat);
    
    std::cout << "AI功能测试完成！" << std::endl;
    return 0;
}