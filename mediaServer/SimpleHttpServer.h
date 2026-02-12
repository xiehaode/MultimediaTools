#ifndef SIMPLEHTTPSERVER_H
#define SIMPLEHTTPSERVER_H

#ifdef SIMPLE_HTTP

#include <string>
#include <map>
#include <functional>
#include <thread>
#include <memory>
#include "DatabaseManager.h"
#include "crypto/CryptoUtils.h"

// 简单的HTTP服务器实现，仅用于基本功能
class SimpleHttpServer {
public:
    static SimpleHttpServer& getInstance();
    
    bool start(const std::string& host, int port);
    void stop();
    bool isRunning() const { return running_; }
    
    void setJwtSecret(const std::string& secret) { jwtSecret_ = secret; }
    void setTokenExpirationHours(int hours) { tokenExpirationHours_ = hours; }

private:
    SimpleHttpServer();
    ~SimpleHttpServer();
    
    SimpleHttpServer(const SimpleHttpServer&) = delete;
    SimpleHttpServer& operator=(const SimpleHttpServer&) = delete;
    
    void runServer();
    void handleRequest(const std::string& method, const std::string& path, 
                     const std::string& body, std::string& response);
    
    // 路由处理
    void handleRegister(const std::string& body, std::string& response);
    void handleLogin(const std::string& body, std::string& response);
    void handleValidateToken(const std::string& headers, std::string& response);
    void handleSaveCommand(const std::string& headers, const std::string& body, std::string& response);
    void handleGetUserCommands(const std::string& headers, std::string& response);
    
    // 工具函数
    std::string createJsonResponse(bool success, const std::string& data = "", const std::string& error = "");
    std::string generateToken(const std::string& username, int userId);
    bool validateToken(const std::string& token);
    std::string extractTokenFromHeader(const std::string& headers);
    
    std::unique_ptr<std::thread> serverThread_;
    bool running_;
    std::string host_;
    int port_;
    std::string jwtSecret_;
    int tokenExpirationHours_;
};

#endif // SIMPLE_HTTP
#endif // SIMPLEHTTPSERVER_H