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
        
        // 密码加密（这里应该获取用户的salt然后哈希）
        auto& db = DatabaseManager::getInstance();
        std::string message;
        User user;
        
        // 简化处理，实际应该先获取用户的salt
        auto& crypto = CryptoUtils::getInstance();
        std::string passwordHash = crypto.sha256(password + username); // 简化版
        
        if (db.authenticateUser(username, passwordHash, user, message)) {
            std::string token = generateToken(user);
            
            json::value responseData;
            responseData[U("token")] = json::value::string(utility::conversions::to_string_t(token));
            responseData[U("user")] = userToJson(user);
            
            sendResponse(request, createSuccessResponse(responseData));
            logRequest(request, "login", true);
            
            // 记录登录日志
            db.logLoginAttempt(username, getClientIP(request), getUserAgent(request), true);
        } else {
            sendError(request, message, status_codes::Unauthorized);
            logRequest(request, "login", false);
            
            db.logLoginAttempt(username, getClientIP(request), getUserAgent(request), false, message);
        }
    } catch (const std::exception& e) {
        sendError(request, std::string("Login failed: ") + e.what());
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
        std::string authHeader = request.headers().find(U("Authorization"))->second;
        if (authHeader.empty() || authHeader.find(U("Bearer ")) != 0) {
            sendError(request, "Missing or invalid authorization header", status_codes::Unauthorized);
            return;
        }
        
        std::string token = utility::conversions::to_utf8string(authHeader.substr(7));
        User user;
        
        if (validateToken(token, user)) {
            json::value responseData;
            responseData[U("user")] = userToJson(user);
            sendResponse(request, createSuccessResponse(responseData));
        } else {
            sendError(request, "Invalid token", status_codes::Unauthorized);
        }
    } catch (const std::exception& e) {
        sendError(request, std::string("Token validation failed: ") + e.what());
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
        return false;
    }
    
    std::string authValue = utility::conversions::to_utf8string(authHeader->second);
    if (authValue.find("Bearer ") != 0) {
        return false;
    }
    
    std::string token = authValue.substr(7);
    return validateToken(token, user);
}

std::string HttpServer::generateToken(const User& user) {
    auto& crypto = CryptoUtils::getInstance();
    std::string token = crypto.generateToken(64);
    
    auto& db = DatabaseManager::getInstance();
    int sessionId;
    db.createSession(user.id, token, "", "", sessionId); // IP和User-Agent在认证时记录
    
    return token;
}

bool HttpServer::validateToken(const std::string& token, User& user) {
    auto& db = DatabaseManager::getInstance();
    return db.validateSession(token, user);
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
    jsonUser[U("username")] = json::value::string(utility::conversions::to_string_t(user.username));
    jsonUser[U("email")] = json::value::string(utility::conversions::to_string_t(user.email));
    jsonUser[U("role")] = json::value::string(utility::conversions::to_string_t(user.role));
    jsonUser[U("isActive")] = json::value::boolean(user.isActive);
    jsonUser[U("lastLogin")] = json::value::string(utility::conversions::to_string_t(user.lastLogin));
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
    
    sendResponse(request, createSuccessResponse(responseData));
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