#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include <thread>
#include <functional>
#include <map>
#include <memory>

#ifdef SIMPLE_HTTP
#include "SimpleHttpServer.h"
#else
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
#endif

#include "DatabaseManager.h"
#include "crypto/CryptoUtils.h"

class HttpServer {
public:
    static HttpServer& getInstance();
    
    // 启动和停止服务器
    bool start(const std::string& host, int port);
    void stop();
    bool isRunning() const { return running_; }
    
    // 服务器配置
    void setJwtSecret(const std::string& secret) { jwtSecret_ = secret; }
    void setTokenExpirationHours(int hours) { tokenExpirationHours_ = hours; }
    
private:
    HttpServer();
    ~HttpServer();
    
    // 禁用拷贝构造和赋值
    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    
    // HTTP服务器
    std::unique_ptr<http_listener> listener_;
    std::thread serverThread_;
    bool running_;
    
    // 安全配置
    std::string jwtSecret_;
    int tokenExpirationHours_;
    
    // 路由处理
    void setupRoutes();
    
    // 认证相关API
    void handleRegister(const http_request& request);
    void handleLogin(const http_request& request);
    void handleLogout(const http_request& request);
    void handleValidateToken(const http_request& request);
    
    // 用户相关API
    void handleGetUserProfile(const http_request& request);
    void handleUpdateUserProfile(const http_request& request);
    
    // 工具函数
    bool authenticateRequest(const http_request& request, User& user);
    std::string generateToken(const User& user);
    std::string generateTokenWithDetails(const User& user, const std::string& clientIP, const std::string& userAgent);
    bool validateToken(const std::string& token, User& user);
    
    // JSON处理辅助函数
    json::value createErrorResponse(const std::string& message, int code = 400);
    json::value createSuccessResponse(const json::value& data = json::value::null());
    json::value userToJson(const User& user);
    
    // 请求解析辅助函数
    bool parseJsonBody(const http_request& request, json::value& body);
    bool extractStringParam(const json::value& json, const std::string& key, std::string& value);
    bool extractIntParam(const json::value& json, const std::string& key, int& value);
    
    // 响应辅助函数
    void sendResponse(const http_request& request, const json::value& response, 
                     status_code status = status_codes::OK);
    void sendError(const http_request& request, const std::string& message, 
                   status_code status = status_codes::BadRequest);
    
    // 日志记录
    void logRequest(const http_request& request, const std::string& operation, bool success);
    std::string getClientIP(const http_request& request);
    std::string getUserAgent(const http_request& request);
};

#endif // HTTPSERVER_H