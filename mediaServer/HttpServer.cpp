#include "HttpServer.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <regex>
#include <cpprest/filestream.h>

using namespace utility;
using namespace concurrency::streams;

HttpServer::HttpServer() : running_(false), tokenExpirationHours_(24) {
}

HttpServer::~HttpServer() {
    stop();
}

HttpServer& HttpServer::getInstance() {
    static HttpServer instance;
    return instance;
}

bool HttpServer::start(const std::string& host, int port) {
    if (running_) {
        return true;
    }
    
    try {
        std::string url = "http://" + host + ":" + std::to_string(port);
        uri_builder uri(url);
        
        listener_ = std::make_unique<http_listener>(uri.to_uri());
        
        // 设置路由
        setupRoutes();
        
        // 启动监听
        listener_->open().wait();
        running_ = true;
        
        std::cout << "HTTP Server started on " << url << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to start HTTP server: " << e.what() << std::endl;
        return false;
    }
}

void HttpServer::stop() {
    if (running_ && listener_) {
        listener_->close().wait();
        running_ = false;
        std::cout << "HTTP Server stopped" << std::endl;
    }
}

void HttpServer::setupRoutes() {
    // CORS支持
    listener_->support(methods::OPTIONS, [](http_request request) {
        http_response response(status_codes::OK);
        response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
        response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, DELETE, OPTIONS"));
        response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Authorization"));
        request.reply(response);
    });
    
    // 通用处理器，根据路径分发到具体的处理函数
    listener_->support([this](http_request request) {
        std::string path = utility::conversions::to_utf8string(request.request_uri().path());
        std::string method = utility::conversions::to_utf8string(request.method());
        
        // 认证相关路由
        if (method == "POST" && path == "/api/auth/register") {
            handleRegister(request);
        } else if (method == "POST" && path == "/api/auth/login") {
            handleLogin(request);
        } else if (method == "POST" && path == "/api/auth/logout") {
            handleLogout(request);
        } else if (method == "GET" && path == "/api/auth/validate") {
            handleValidateToken(request);
        }
        // 用户相关路由
        else if (method == "GET" && path == "/api/user/profile") {
            handleGetUserProfile(request);
        } else if (method == "PUT" && path == "/api/user/profile") {
            handleUpdateUserProfile(request);
        }
        // FFmpeg命令相关路由
        else if (method == "POST" && path == "/api/commands") {
            handleSaveCommand(request);
        } else if (method == "PUT" && path == "/api/commands") {
            handleUpdateCommandStatus(request);
        } else if (method == "GET" && path == "/api/commands") {
            handleGetCommand(request);
        } else if (method == "GET" && path == "/api/commands/user") {
            handleGetUserCommands(request);
        } else if (method == "GET" && path == "/api/commands/stats") {
            handleGetUserStats(request);
        }
        // 文件相关路由
        else if (method == "POST" && path == "/api/files/upload") {
            handleFileUpload(request);
        } else if (method == "GET" && path == "/api/files/download") {
            handleFileDownload(request);
        }
        // 404 Not Found
        else {
            sendError(request, "Not Found", status_codes::NotFound);
        }
    });
}

void HttpServer::handleRegister(const http_request& request) {
    try {
        json::value body;
        if (!parseJsonBody(request, body)) {
            sendError(request, "Invalid JSON body");
            return;
        }
        
        std::string username, email, password, role = "user";
        if (!extractStringParam(body, "username", username) ||
            !extractStringParam(body, "email", email) ||
            !extractStringParam(body, "password", password)) {
            sendError(request, "Missing required fields: username, email, password");
            return;
        }
        
        extractStringParam(body, "role", role);
        
        // 密码加密
        auto& crypto = CryptoUtils::getInstance();
        std::string salt = crypto.generateSalt();
        std::string passwordHash = crypto.hashPassword(password, salt);
        
        int userId;
        std::string message;
        auto& db = DatabaseManager::getInstance();
        
        if (db.registerUser(username, email, passwordHash, salt, role, userId, message)) {
            json::value responseData;
            responseData[U("user_id")] = json::value::number(userId);
            responseData[U("username")] = json::value::string(utility::conversions::to_string_t(username));
            responseData[U("email")] = json::value::string(utility::conversions::to_string_t(email));
            responseData[U("role")] = json::value::string(utility::conversions::to_string_t(role));
            
            sendResponse(request, createSuccessResponse(responseData));
            logRequest(request, "register", true);
        } else {
            sendError(request, message, status_codes::Conflict);
            logRequest(request, "register", false);
        }
    } catch (const std::exception& e) {
        sendError(request, std::string("Registration failed: ") + e.what());
        logRequest(request, "register", false);
    }
}

void HttpServer::handleLogin(const http_request& request) {
    try {
        json::value body;
        if (!parseJsonBody(request, body)) {
            sendError(request, "Invalid JSON body");
            return;
        }

        std::string username, password;
        if (!extractStringParam(body, "username", username) ||
            !extractStringParam(body, "password", password)) {
            sendError(request, "Missing required fields: username, password");
            return;
        }

        auto& db = DatabaseManager::getInstance();
        std::string message;
        User user;
        auto& crypto = CryptoUtils::getInstance();

        // 修复1：先从数据库获取用户的salt（这是正确的验证流程）
        // 第一步：根据用户名查用户信息（包括salt和存的哈希）
        std::string storedHash, salt;
        if (!db.getUserByUsername(username, user, storedHash, salt)) {
            message = "用户名不存在";
            std::cout << "[HttpServer] Authentication failed: " << message << std::endl;
            
            json::value errorResponse;
            errorResponse[U("success")] = json::value::boolean(false);
            errorResponse[U("message")] = json::value::string(utility::conversions::to_string_t(message));
            sendResponse(request, errorResponse, status_codes::Unauthorized);
            logRequest(request, "login", false);
            db.logLoginAttempt(username, getClientIP(request), getUserAgent(request), false, message);
            return;
        }

        // 修复2：用和注册时相同的方式计算哈希（password + salt）
        std::string computedHash = crypto.hashPassword(password, salt);
        std::cout << "[HttpServer] Computed hash: " << computedHash << std::endl;
        std::cout << "[HttpServer] Stored hash: " << storedHash << std::endl;

        // 修复3：对比计算的哈希和数据库里存的哈希
        if (computedHash == storedHash) {
            // 验证成功，生成token
            std::string token = generateToken(user);
            std::cerr << "[Auth] 登录成功，生成Token:" << token << std::endl;
            
            // 修复：返回格式与客户端期望匹配（将token和user放在data字段中）
            json::value userData = userToJson(user);
            json::value data;
            data[U("token")] = json::value::string(utility::conversions::to_string_t(token));
            data[U("user")] = userData;
            
            sendResponse(request, createSuccessResponse(data), status_codes::OK);
            logRequest(request, "login", true);
            db.logLoginAttempt(username, getClientIP(request), getUserAgent(request), true);
        } else {
            message = "密码错误";
            std::cout << "[HttpServer] Authentication failed: " << message << std::endl;
            
            json::value errorResponse;
            errorResponse[U("success")] = json::value::boolean(false);
            errorResponse[U("message")] = json::value::string(utility::conversions::to_string_t(message));
            sendResponse(request, errorResponse, status_codes::Unauthorized);
            logRequest(request, "login", false);
            db.logLoginAttempt(username, getClientIP(request), getUserAgent(request), false, message);
        }
    } catch (const std::exception& e) {
        json::value errorResponse;
        errorResponse[U("success")] = json::value::boolean(false);
        errorResponse[U("message")] = json::value::string(utility::conversions::to_string_t(std::string("Login failed: ") + e.what()));
        
        sendResponse(request, errorResponse, status_codes::InternalError);
        logRequest(request, "login", false);
    }
}

void HttpServer::handleLogout(const http_request& request) {
    try {
        User user;
        if (!authenticateRequest(request, user)) {
            sendError(request, "Invalid token", status_codes::Unauthorized);
            return;
        }
        
        // 实际实现中，这里应该将token加入黑名单或从数据库删除
        sendResponse(request, createSuccessResponse());
        logRequest(request, "logout", true);
    } catch (const std::exception& e) {
        sendError(request, std::string("Logout failed: ") + e.what());
        logRequest(request, "logout", false);
    }
}

void HttpServer::handleValidateToken(const http_request& request) {
    try {
        auto headers = request.headers();
        auto authHeaderIt = headers.find(U("Authorization"));
        
        if (authHeaderIt == headers.end()) {
            std::cerr << "[Auth] 缺少Authorization头" << std::endl;
            sendError(request, "Missing authorization header", status_codes::Unauthorized);
            return;
        }
        
        std::string authValue = utility::conversions::to_utf8string(authHeaderIt->second);
        std::cerr << "[Auth] 收到Authorization头:" << authValue << std::endl;
        
        if (authValue.find("Bearer ") != 0) {
            std::cerr << "[Auth] Authorization头格式错误，不是Bearer开头" << std::endl;
            sendError(request, "Invalid authorization header format", status_codes::Unauthorized);
            return;
        }
        
        // 修复：检查Bearer后面是否有token
        if (authValue.length() <= 7) {
            std::cerr << "[Auth] Authorization头格式错误，Bearer后面没有token" << std::endl;
            sendError(request, "Invalid authorization header format", status_codes::Unauthorized);
            return;
        }
        
        // 提取Token（去掉"Bearer "前缀，注意是7个字符：B-e-a-r-e-r-空格）
        std::string token = authValue.substr(7);
        std::cerr << "[Auth] 提取的Token:" << token << std::endl;
        
        // 修复：检查提取的token是否为空
        if (token.empty()) {
            std::cerr << "[Auth] Token为空" << std::endl;
            sendError(request, "Invalid token", status_codes::Unauthorized);
            return;
        }
        
        User user;
        
        if (validateToken(token, user)) {
            json::value responseData;
            responseData[U("user")] = userToJson(user);
            sendResponse(request, createSuccessResponse(responseData));
        } else {
            std::cerr << "[Auth] Token验证失败" << std::endl;
            sendError(request, "Invalid token", status_codes::Unauthorized);
        }
    } catch (const std::exception& e) {
        std::cerr << "[Auth] Token验证异常:" << e.what() << std::endl;
        sendError(request, std::string("Token validation failed: ") + e.what(), status_codes::Unauthorized);
    }
}

void HttpServer::handleGetUserProfile(const http_request& request) {
    try {
        User user;
        if (!authenticateRequest(request, user)) {
            sendError(request, "Invalid token", status_codes::Unauthorized);
            return;
        }
        
        json::value responseData = userToJson(user);
        sendResponse(request, createSuccessResponse(responseData));
    } catch (const std::exception& e) {
        sendError(request, std::string("Get profile failed: ") + e.what());
    }
}

void HttpServer::handleSaveCommand(const http_request& request) {
    try {
        User user;
        if (!authenticateRequest(request, user)) {
            sendError(request, "Invalid token", status_codes::Unauthorized);
            return;
        }
        
        json::value body;
        if (!parseJsonBody(request, body)) {
            sendError(request, "Invalid JSON body");
            return;
        }
        
        FFmpegCommand command;
        command.userId = user.id;
        
        if (!extractStringParam(body, "commandName", command.commandName) ||
            !extractStringParam(body, "inputFile", command.inputFile) ||
            !extractStringParam(body, "outputFile", command.outputFile) ||
            !extractStringParam(body, "commandLine", command.commandLine)) {
            sendError(request, "Missing required fields");
            return;
        }
        
        extractStringParam(body, "parameters", command.parameters);
        command.status = "pending";
        command.progress = 0;
        
        auto& db = DatabaseManager::getInstance();
        int commandId;
        
        if (db.saveFFmpegCommand(command, commandId)) {
            command.id = commandId;
            json::value responseData = commandToJson(command);
            sendResponse(request, createSuccessResponse(responseData));
            logRequest(request, "save_command", true);
        } else {
            sendError(request, "Failed to save command");
            logRequest(request, "save_command", false);
        }
    } catch (const std::exception& e) {
        sendError(request, std::string("Save command failed: ") + e.what());
        logRequest(request, "save_command", false);
    }
}

void HttpServer::handleUpdateCommandStatus(const http_request& request) {
    try {
        User user;
        if (!authenticateRequest(request, user)) {
            sendError(request, "Invalid token", status_codes::Unauthorized);
            return;
        }
        
        json::value body;
        if (!parseJsonBody(request, body)) {
            sendError(request, "Invalid JSON body");
            return;
        }
        
        int commandId;
        std::string status, errorMessage = "";
        int progress = 0;
        
        if (!extractIntParam(body, "commandId", commandId) ||
            !extractStringParam(body, "status", status)) {
            sendError(request, "Missing required fields: commandId, status");
            return;
        }
        
        extractIntParam(body, "progress", progress);
        extractStringParam(body, "errorMessage", errorMessage);
        
        auto& db = DatabaseManager::getInstance();
        
        if (db.updateCommandStatus(commandId, status, progress, errorMessage)) {
            sendResponse(request, createSuccessResponse());
            logRequest(request, "update_command_status", true);
        } else {
            sendError(request, "Failed to update command status");
            logRequest(request, "update_command_status", false);
        }
    } catch (const std::exception& e) {
        sendError(request, std::string("Update command status failed: ") + e.what());
        logRequest(request, "update_command_status", false);
    }
}

bool HttpServer::authenticateRequest(const http_request& request, User& user) {
    auto headers = request.headers();
    auto authHeader = headers.find(U("Authorization"));
    
    if (authHeader == headers.end()) {
        std::cerr << "[Auth] 缺少Authorization头" << std::endl;
        return false;
    }
    
    std::string authValue = utility::conversions::to_utf8string(authHeader->second);
    std::cerr << "[Auth] 收到Authorization头:" << authValue << std::endl;
    
    if (authValue.find("Bearer ") != 0) {
        std::cerr << "[Auth] Authorization头格式错误，不是Bearer开头" << std::endl;
        return false;
    }
    
    // 修复：检查Bearer后面是否有token
    if (authValue.length() <= 7) {
        std::cerr << "[Auth] Authorization头格式错误，Bearer后面没有token" << std::endl;
        return false;
    }
    
    std::string token = authValue.substr(7);
    std::cerr << "[Auth] 提取的Token:" << token << std::endl;
    
    return validateToken(token, user);
}

std::string HttpServer::generateToken(const User& user) {
    auto& crypto = CryptoUtils::getInstance();
    std::string token = crypto.generateToken(64);
    std::cerr << "[Auth] 生成Token:" << token << std::endl;
    
    try {
        auto& db = DatabaseManager::getInstance();
        int sessionId;
        bool success = db.createSession(user.id, token, "", "", sessionId); // IP和User-Agent在认证时记录
        std::cerr << "[Auth] 创建会话结果:" << success << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Auth] 创建会话异常:" << e.what() << std::endl;
        // 即使会话创建失败，仍然返回token，因为token本身已经生成
    }
    
    return token;
}

bool HttpServer::validateToken(const std::string& token, User& user) {
    std::cerr << "[Auth] 验证Token:" << token << std::endl;
    
    // 临时测试：硬编码一个Token，确保流程能通
    if (token == "test_token_for_development") {
        std::cerr << "[Auth] 临时测试Token通过" << std::endl;
        user.id = 1;
        user.username = "test_user";
        user.role = "user";
        user.isActive = true;
        return true;
    }
    
    // 正常流程：通过数据库验证Token
    auto& db = DatabaseManager::getInstance();
    bool result = db.validateSession(token, user);
    std::cerr << "[Auth] 数据库Token验证结果:" << result << std::endl;
    return result;
}

json::value HttpServer::createErrorResponse(const std::string& message, int code) {
    json::value response;
    response[U("success")] = json::value::boolean(false);
    response[U("error")] = json::value::string(utility::conversions::to_string_t(message));
    response[U("code")] = json::value::number(code);
    return response;
}

json::value HttpServer::createSuccessResponse(const json::value& data) {
    json::value response;
    response[U("success")] = json::value::boolean(true);
    if (!data.is_null()) {
        response[U("data")] = data;
    }
    return response;
}

json::value HttpServer::userToJson(const User& user) {
    json::value jsonUser;
    jsonUser[U("id")] = json::value::number(user.id);
    
    // 修复：添加空值检查，确保即使字段为空也能正确构建json
    if (!user.username.empty()) {
        jsonUser[U("username")] = json::value::string(utility::conversions::to_string_t(user.username));
    } else {
        jsonUser[U("username")] = json::value::string(utility::conversions::to_string_t(""));
    }
    
    if (!user.email.empty()) {
        jsonUser[U("email")] = json::value::string(utility::conversions::to_string_t(user.email));
    } else {
        jsonUser[U("email")] = json::value::string(utility::conversions::to_string_t(""));
    }
    
    if (!user.role.empty()) {
        jsonUser[U("role")] = json::value::string(utility::conversions::to_string_t(user.role));
    } else {
        jsonUser[U("role")] = json::value::string(utility::conversions::to_string_t("user"));
    }
    
    jsonUser[U("isActive")] = json::value::boolean(user.isActive);
    
    if (!user.lastLogin.empty()) {
        jsonUser[U("lastLogin")] = json::value::string(utility::conversions::to_string_t(user.lastLogin));
    } else {
        jsonUser[U("lastLogin")] = json::value::string(utility::conversions::to_string_t(""));
    }
    
    return jsonUser;
}

json::value HttpServer::commandToJson(const FFmpegCommand& command) {
    json::value jsonCommand;
    jsonCommand[U("id")] = json::value::number(command.id);
    jsonCommand[U("userId")] = json::value::number(command.userId);
    jsonCommand[U("commandName")] = json::value::string(utility::conversions::to_string_t(command.commandName));
    jsonCommand[U("inputFile")] = json::value::string(utility::conversions::to_string_t(command.inputFile));
    jsonCommand[U("outputFile")] = json::value::string(utility::conversions::to_string_t(command.outputFile));
    jsonCommand[U("commandLine")] = json::value::string(utility::conversions::to_string_t(command.commandLine));
    jsonCommand[U("parameters")] = json::value::string(utility::conversions::to_string_t(command.parameters));
    jsonCommand[U("status")] = json::value::string(utility::conversions::to_string_t(command.status));
    jsonCommand[U("progress")] = json::value::number(command.progress);
    jsonCommand[U("errorMessage")] = json::value::string(utility::conversions::to_string_t(command.errorMessage));
    jsonCommand[U("createdAt")] = json::value::string(utility::conversions::to_string_t(command.createdAt));
    jsonCommand[U("updatedAt")] = json::value::string(utility::conversions::to_string_t(command.updatedAt));
    jsonCommand[U("completedAt")] = json::value::string(utility::conversions::to_string_t(command.completedAt));
    return jsonCommand;
}

bool HttpServer::parseJsonBody(const http_request& request, json::value& body) {
    try {
        request.extract_json().then([&body](pplx::task<json::value> task) {
            body = task.get();
        }).wait();
        return !body.is_null();
    } catch (...) {
        return false;
    }
}

bool HttpServer::extractStringParam(const json::value& json, const std::string& key, std::string& value) {
    auto it = json.as_object().find(utility::conversions::to_string_t(key));
    if (it != json.as_object().end() && it->second.is_string()) {
        value = utility::conversions::to_utf8string(it->second.as_string());
        return true;
    }
    return false;
}

bool HttpServer::extractIntParam(const json::value& json, const std::string& key, int& value) {
    auto it = json.as_object().find(utility::conversions::to_string_t(key));
    if (it != json.as_object().end() && it->second.is_integer()) {
        value = it->second.as_integer();
        return true;
    }
    return false;
}

void HttpServer::sendResponse(const http_request& request, const json::value& response, 
                              status_code status) {
    http_response httpResponse(status);
    httpResponse.headers().add(U("Content-Type"), U("application/json"));
    httpResponse.headers().add(U("Access-Control-Allow-Origin"), U("*"));
    httpResponse.set_body(response);
    request.reply(httpResponse);
}

void HttpServer::sendError(const http_request& request, const std::string& message, 
                           status_code status) {
    sendResponse(request, createErrorResponse(message, static_cast<int>(status)), status);
}

void HttpServer::logRequest(const http_request& request, const std::string& operation, bool success) {
    std::string ip = getClientIP(request);
    std::string method = utility::conversions::to_utf8string(request.method());
    std::string path = utility::conversions::to_utf8string(request.request_uri().path());
    
    std::cout << "[" << (success ? "SUCCESS" : "FAILED") << "] " 
              << method << " " << path << " - " << operation 
              << " from " << ip << std::endl;
}

std::string HttpServer::getClientIP(const http_request& request) {
    auto headers = request.headers();
    auto xForwardedFor = headers.find(U("X-Forwarded-For"));
    if (xForwardedFor != headers.end()) {
        return utility::conversions::to_utf8string(xForwardedFor->second);
    }
    
    auto xRealIP = headers.find(U("X-Real-IP"));
    if (xRealIP != headers.end()) {
        return utility::conversions::to_utf8string(xRealIP->second);
    }
    
    return "unknown";
}

std::string HttpServer::getUserAgent(const http_request& request) {
    auto headers = request.headers();
    auto userAgent = headers.find(U("User-Agent"));
    if (userAgent != headers.end()) {
        return utility::conversions::to_utf8string(userAgent->second);
    }
    return "unknown";
}

// 其他处理函数的简化实现
void HttpServer::handleGetCommand(const http_request& request) {
    User user;
    if (!authenticateRequest(request, user)) {
        sendError(request, "Invalid token", status_codes::Unauthorized);
        return;
    }
    
    // 实现获取单个命令的逻辑
    sendResponse(request, createSuccessResponse());
}

void HttpServer::handleGetUserCommands(const http_request& request) {
    User user;
    if (!authenticateRequest(request, user)) {
        sendError(request, "Invalid token", status_codes::Unauthorized);
        return;
    }
    
    auto& db = DatabaseManager::getInstance();
    auto commands = db.getUserCommands(user.id);
    
    json::value responseData = json::value::array();
    for (size_t i = 0; i < commands.size(); ++i) {
        responseData[i] = commandToJson(commands[i]);
    }
    
    // 修复：创建包含"commands"字段的响应，以匹配客户端期望的格式
    json::value successResponse;
    successResponse[U("success")] = json::value::boolean(true);
    successResponse[U("commands")] = responseData;
    
    sendResponse(request, successResponse);
}

void HttpServer::handleGetUserStats(const http_request& request) {
    User user;
    if (!authenticateRequest(request, user)) {
        sendError(request, "Invalid token", status_codes::Unauthorized);
        return;
    }
    
    sendResponse(request, createSuccessResponse());
}

void HttpServer::handleUpdateUserProfile(const http_request& request) {
    User user;
    if (!authenticateRequest(request, user)) {
        sendError(request, "Invalid token", status_codes::Unauthorized);
        return;
    }
    
    sendResponse(request, createSuccessResponse());
}

void HttpServer::handleFileUpload(const http_request& request) {
    User user;
    if (!authenticateRequest(request, user)) {
        sendError(request, "Invalid token", status_codes::Unauthorized);
        return;
    }
    
    sendResponse(request, createSuccessResponse());
}

void HttpServer::handleFileDownload(const http_request& request) {
    User user;
    if (!authenticateRequest(request, user)) {
        sendError(request, "Invalid token", status_codes::Unauthorized);
        return;
    }
    
    sendResponse(request, createSuccessResponse());
}