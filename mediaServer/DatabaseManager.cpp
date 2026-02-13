#include "DatabaseManager.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <cstring>

DatabaseManager::DatabaseManager() : connection_(nullptr), connected_(false) {
    connection_ = mysql_init(nullptr);
    if (!connection_) {
        std::cerr << "Failed to initialize MySQL connection" << std::endl;
    }
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::connect(const std::string& host, const std::string& user, 
                              const std::string& password, const std::string& database, int port) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (connected_) {
        return true;
    }
    
    if (!mysql_real_connect(connection_, host.c_str(), user.c_str(), 
                            password.c_str(), database.c_str(), port, nullptr, 0)) {
        logError("connect", getMySQLError());
        return false;
    }
    
    // 设置字符集为UTF-8
    if (mysql_set_character_set(connection_, "utf8mb4") != 0) {
        logError("set_character_set", getMySQLError());
        disconnect();
        return false;
    }
    
    connected_ = true;
    return true;
}

void DatabaseManager::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (connection_) {
        mysql_close(connection_);
        connection_ = nullptr;
    }
    connected_ = false;
}

bool DatabaseManager::isConnected() const {
    return connected_ && connection_;
}

bool DatabaseManager::registerUser(const std::string& username, const std::string& email,
                                   const std::string& passwordHash, const std::string& salt,
                                   const std::string& role, int& userId, std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        message = "数据库连接失败";
        return false;
    }
    
    std::string query = "CALL RegisterUser(?, ?, ?, ?, ?, @user_id, @status, @message)";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        message = "预处理语句创建失败";
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        message = "预处理语句准备失败: " + std::string(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));
    
    // 绑定输入参数
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)username.c_str();
    bind[0].buffer_length = username.length();
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)email.c_str();
    bind[1].buffer_length = email.length();
    
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)passwordHash.c_str();
    bind[2].buffer_length = passwordHash.length();
    
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void*)salt.c_str();
    bind[3].buffer_length = salt.length();
    
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void*)role.c_str();
    bind[4].buffer_length = role.length();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        message = "参数绑定失败: " + std::string(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        message = "语句执行失败: " + std::string(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 处理存储过程可能返回的结果集
    while (mysql_stmt_next_result(stmt) == 0) {
        MYSQL_RES* proc_result = mysql_stmt_result_metadata(stmt);
        if (proc_result) {
            // 消费存储过程中的结果集
            while (mysql_stmt_fetch(stmt) == 0) {
                // 丢弃行数据
            }
            mysql_free_result(proc_result);
        }
    }
    
    mysql_stmt_close(stmt);
    
    // 获取输出参数
    query = "SELECT @user_id, @status, @message";
    if (mysql_query(connection_, query.c_str()) != 0) {
        message = "查询输出参数失败: " + getMySQLError();
        userId = 0;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(connection_);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[1] && row[2]) {
            std::string status = row[1];
            message = row[2];
            
            // 处理用户ID，可能为NULL
            if (row[0] && row[0] != nullptr && strlen(row[0]) > 0) {
                try {
                    userId = std::stoi(row[0]);
                } catch (const std::exception& e) {
                    userId = 0;
                }
            } else {
                userId = 0;
            }
            
            mysql_free_result(result);
            return (status == "success");
        }
        mysql_free_result(result);
    }
    
    message = "获取结果失败";
    userId = 0;
    return false;
}

bool DatabaseManager::registerUser(const std::string& username, const std::string& email,
                                   const std::string& passwordHash, const std::string& salt,
                                   const std::string& role) {
    int userId;
    std::string message;
    return registerUser(username, email, passwordHash, salt, role, userId, message);
}

bool DatabaseManager::authenticateUser(const std::string& username, const std::string& passwordHash,
                                       User& user, std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        message = "数据库连接失败";
        return false;
    }
    
    std::string query = "CALL AuthenticateUser(?, ?, @user_id, @role, @status, @message)";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        message = "预处理语句创建失败";
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        message = "预处理语句准备失败: " + std::string(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)username.c_str();
    bind[0].buffer_length = username.length();
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)passwordHash.c_str();
    bind[1].buffer_length = passwordHash.length();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        message = "参数绑定失败: " + std::string(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        message = "语句执行失败: " + std::string(mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 处理存储过程可能返回的结果集
    while (mysql_stmt_next_result(stmt) == 0) {
        MYSQL_RES* proc_result = mysql_stmt_result_metadata(stmt);
        if (proc_result) {
            // 消费存储过程中的结果集
            while (mysql_stmt_fetch(stmt) == 0) {
                // 丢弃行数据
            }
            mysql_free_result(proc_result);
        }
    }
    
    mysql_stmt_close(stmt);
    
    // 获取输出参数
    query = "SELECT @user_id, @role, @status, @message";
    if (mysql_query(connection_, query.c_str()) != 0) {
        message = "查询输出参数失败: " + getMySQLError();
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(connection_);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[1] && row[2] && row[3]) {
            std::string role = row[1];
            std::string status = row[2];
            message = row[3];
            
            // 处理用户ID，可能为NULL
            int userId = 0;
            if (row[0] && row[0] != nullptr && strlen(row[0]) > 0) {
                try {
                    userId = std::stoi(row[0]);
                } catch (const std::exception& e) {
                    userId = 0;
                }
            }
            
            mysql_free_result(result);
            
            if (status == "success") {
                user.id = userId;
                user.username = username;
                user.role = role;
                user.isActive = true;
                return getUserById(userId, user);
            }
            return false;
        }
        mysql_free_result(result);
    }
    
    message = "获取结果失败";
    return false;
}

bool DatabaseManager::getUserSalt(const std::string& username, std::string& salt, std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    std::string query = "SELECT salt, password_hash FROM users WHERE username = ? LIMIT 1";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)username.c_str();
    bind[0].buffer_length = username.length();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
    if (result) {
        MYSQL_BIND result_bind[2];
        memset(result_bind, 0, sizeof(result_bind));
        
        char salt_str[65], hash_str[256];
        unsigned long salt_len, hash_len;
        
        result_bind[0].buffer_type = MYSQL_TYPE_STRING;
        result_bind[0].buffer = salt_str;
        result_bind[0].buffer_length = sizeof(salt_str);
        result_bind[0].length = &salt_len;
        
        result_bind[1].buffer_type = MYSQL_TYPE_STRING;
        result_bind[1].buffer = hash_str;
        result_bind[1].buffer_length = sizeof(hash_str);
        result_bind[1].length = &hash_len;
        
        if (mysql_stmt_bind_result(stmt, result_bind) == 0 && mysql_stmt_fetch(stmt) == 0) {
            salt = std::string(salt_str, salt_len);
            hash = std::string(hash_str, hash_len);
            
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return true;
        }
        
        mysql_free_result(result);
    }
    
    mysql_stmt_close(stmt);
    return false;
}

bool DatabaseManager::getUserById(int userId, User& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    std::string query = "SELECT id, username, email, role, is_active, last_login FROM users WHERE id = ?";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &userId;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
    if (result) {
        MYSQL_FIELD* fields = mysql_fetch_fields(result);
        int field_count = mysql_num_fields(result);
        
        MYSQL_BIND result_bind[6];
        memset(result_bind, 0, sizeof(result_bind));
        
        int id, is_active;
        char username[51], email[101], role[11], last_login[20];
        unsigned long username_len, email_len, role_len, last_login_len;
        
        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &id;
        result_bind[1].buffer_type = MYSQL_TYPE_STRING;
        result_bind[1].buffer = username;
        result_bind[1].buffer_length = sizeof(username);
        result_bind[1].length = &username_len;
        result_bind[2].buffer_type = MYSQL_TYPE_STRING;
        result_bind[2].buffer = email;
        result_bind[2].buffer_length = sizeof(email);
        result_bind[2].length = &email_len;
        result_bind[3].buffer_type = MYSQL_TYPE_STRING;
        result_bind[3].buffer = role;
        result_bind[3].buffer_length = sizeof(role);
        result_bind[3].length = &role_len;
        result_bind[4].buffer_type = MYSQL_TYPE_TINY;
        result_bind[4].buffer = &is_active;
        result_bind[5].buffer_type = MYSQL_TYPE_STRING;
        result_bind[5].buffer = last_login;
        result_bind[5].buffer_length = sizeof(last_login);
        result_bind[5].length = &last_login_len;
        
        if (mysql_stmt_bind_result(stmt, result_bind) == 0 && mysql_stmt_fetch(stmt) == 0) {
            user.id = id;
            user.username = std::string(username, username_len);
            user.email = std::string(email, email_len);
            user.role = std::string(role, role_len);
            user.isActive = (is_active != 0);
            user.lastLogin = std::string(last_login, last_login_len);
            
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return true;
        }
        
        mysql_free_result(result);
    }
    
    mysql_stmt_close(stmt);
    return false;
}

bool DatabaseManager::createSession(int userId, const std::string& sessionToken,
                                   const std::string& ipAddress, const std::string& userAgent,
                                   int& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    // 计算过期时间（24小时后）
    auto now = std::time(nullptr);
    auto expiresAt = now + 24 * 60 * 60;
    
    std::string expiresStr = std::to_string(expiresAt);
    
    std::string query = "CALL CreateUserSession(?, ?, ?, ?, @session_id, @status)";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &userId;
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)sessionToken.c_str();
    bind[1].buffer_length = sessionToken.length();
    
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)expiresStr.c_str();
    bind[2].buffer_length = expiresStr.length();
    
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void*)ipAddress.c_str();
    bind[3].buffer_length = ipAddress.length();
    
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void*)userAgent.c_str();
    bind[4].buffer_length = userAgent.length();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    mysql_stmt_close(stmt);
    
    // 获取结果
    query = "SELECT @session_id, @status";
    MYSQL_RES* result = mysql_store_result(connection_);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0] && row[1]) {
            sessionId = std::stoi(row[0]);
            std::string status = row[1];
            
            mysql_free_result(result);
            return (status == "success");
        }
        mysql_free_result(result);
    }
    
    return false;
}

bool DatabaseManager::validateSession(const std::string& sessionToken, User& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    std::string query = "CALL ValidateSession(?, @user_id, @username, @role, @status)";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)sessionToken.c_str();
    bind[0].buffer_length = sessionToken.length();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    mysql_stmt_close(stmt);
    
    // 获取结果
    query = "SELECT @user_id, @username, @role, @status";
    MYSQL_RES* result = mysql_store_result(connection_);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0] && row[1] && row[2] && row[3]) {
            int userId = std::stoi(row[0]);
            std::string username = row[1];
            std::string role = row[2];
            std::string status = row[3];
            
            mysql_free_result(result);
            
            if (status == "success") {
                return getUserById(userId, user);
            }
        }
        mysql_free_result(result);
    }
    
    return false;
}

bool DatabaseManager::saveFFmpegCommand(const FFmpegCommand& command, int& commandId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    std::string query = "CALL SaveFFmpegCommand(?, ?, ?, ?, ?, ?, @command_id, @status)";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[6];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    int userIdCopy = command.userId;
    bind[0].buffer = &userIdCopy;
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)command.commandName.c_str();
    bind[1].buffer_length = command.commandName.length();
    
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)command.inputFile.c_str();
    bind[2].buffer_length = command.inputFile.length();
    
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void*)command.outputFile.c_str();
    bind[3].buffer_length = command.outputFile.length();
    
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void*)command.commandLine.c_str();
    bind[4].buffer_length = command.commandLine.length();
    
    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (void*)command.parameters.c_str();
    bind[5].buffer_length = command.parameters.length();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    mysql_stmt_close(stmt);
    
    // 获取结果
    query = "SELECT @command_id, @status";
    MYSQL_RES* result = mysql_store_result(connection_);
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0] && row[1]) {
            commandId = std::stoi(row[0]);
            std::string status = row[1];
            
            mysql_free_result(result);
            return (status == "success");
        }
        mysql_free_result(result);
    }
    
    return false;
}

std::string DatabaseManager::getMySQLError() {
    if (connection_) {
        return std::string(mysql_error(connection_));
    }
    return "Connection not initialized";
}

void DatabaseManager::logError(const std::string& operation, const std::string& error) {
    std::cerr << "Database error in " << operation << ": " << error << std::endl;
}

User DatabaseManager::parseUserFromRow(const std::vector<std::string>& row) {
    User user;
    if (row.size() >= 6) {
        user.id = std::stoi(row[0]);
        user.username = row[1];
        user.email = row[2];
        user.role = row[3];
        user.isActive = (row[4] == "1");
        user.lastLogin = row[5];
    }
    return user;
}

FFmpegCommand DatabaseManager::parseCommandFromRow(const std::vector<std::string>& row) {
    FFmpegCommand command;
    if (row.size() >= 11) {
        command.id = std::stoi(row[0]);
        command.userId = std::stoi(row[1]);
        command.commandName = row[2];
        command.inputFile = row[3];
        command.outputFile = row[4];
        command.commandLine = row[5];
        command.parameters = row[6];
        command.status = row[7];
        command.progress = std::stoi(row[8]);
        command.errorMessage = row[9];
        command.createdAt = row[10];
    }
    return command;
}

bool DatabaseManager::logLoginAttempt(const std::string& username, const std::string& ipAddress,
                                     const std::string& userAgent, bool success,
                                     const std::string& failureReason) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    std::string query = "CALL LogLoginAttempt(NULL, ?, ?, ?, ?, ?)";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)username.c_str();
    bind[0].buffer_length = username.length();
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)ipAddress.c_str();
    bind[1].buffer_length = ipAddress.length();
    
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)userAgent.c_str();
    bind[2].buffer_length = userAgent.length();
    
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void*)(success ? "success" : "failed");
    bind[3].buffer_length = success ? 7 : 6;
    
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void*)failureReason.c_str();
    bind[4].buffer_length = failureReason.length();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    bool result = (mysql_stmt_execute(stmt) == 0);
    mysql_stmt_close(stmt);
    
    return result;
}

void DatabaseManager::cleanupExpiredSessions() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return;
    }
    
    std::string query = "DELETE FROM user_sessions WHERE expires_at < CURRENT_TIMESTAMP";
    if (mysql_query(connection_, query.c_str()) != 0) {
        logError("cleanup_expired_sessions", getMySQLError());
    }
}

bool DatabaseManager::updateCommandStatus(int commandId, const std::string& status, 
                                       int progress, const std::string& errorMessage) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    std::string query = "UPDATE ffmpeg_commands SET status = ?, progress = ?, error_message = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[4];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)status.c_str();
    bind[0].buffer_length = status.length();
    
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &progress;
    
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)errorMessage.c_str();
    bind[2].buffer_length = errorMessage.length();
    
    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = &commandId;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    bool result = (mysql_stmt_execute(stmt) == 0);
    mysql_stmt_close(stmt);
    
    return result;
}

std::vector<FFmpegCommand> DatabaseManager::getUserCommands(int userId, int limit, int offset) {
    std::vector<FFmpegCommand> commands;
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return commands;
    }
    
    std::string query = "SELECT id, user_id, command_name, input_file, output_file, command_line, parameters, status, progress, error_message, created_at, updated_at, completed_at FROM ffmpeg_commands WHERE user_id = ? ORDER BY created_at DESC LIMIT ? OFFSET ?";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return commands;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return commands;
    }
    
    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &userId;
    
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &limit;
    
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &offset;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return commands;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return commands;
    }
    
    MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
    if (result) {
        MYSQL_BIND result_bind[13];
        memset(result_bind, 0, sizeof(result_bind));
        
        int id, user_id, progress;
        char command_name[101], input_file[501], output_file[501], command_line[2001], parameters[2001], status[51], error_message[501], created_at[20], updated_at[20], completed_at[20];
        unsigned long command_name_len, input_file_len, output_file_len, command_line_len, parameters_len, status_len, error_message_len, created_at_len, updated_at_len, completed_at_len;
        
        // Bind all result columns
        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &id;
        result_bind[1].buffer_type = MYSQL_TYPE_LONG;
        result_bind[1].buffer = &user_id;
        result_bind[2].buffer_type = MYSQL_TYPE_STRING;
        result_bind[2].buffer = command_name;
        result_bind[2].buffer_length = sizeof(command_name);
        result_bind[2].length = &command_name_len;
        result_bind[3].buffer_type = MYSQL_TYPE_STRING;
        result_bind[3].buffer = input_file;
        result_bind[3].buffer_length = sizeof(input_file);
        result_bind[3].length = &input_file_len;
        result_bind[4].buffer_type = MYSQL_TYPE_STRING;
        result_bind[4].buffer = output_file;
        result_bind[4].buffer_length = sizeof(output_file);
        result_bind[4].length = &output_file_len;
        result_bind[5].buffer_type = MYSQL_TYPE_STRING;
        result_bind[5].buffer = command_line;
        result_bind[5].buffer_length = sizeof(command_line);
        result_bind[5].length = &command_line_len;
        result_bind[6].buffer_type = MYSQL_TYPE_STRING;
        result_bind[6].buffer = parameters;
        result_bind[6].buffer_length = sizeof(parameters);
        result_bind[6].length = &parameters_len;
        result_bind[7].buffer_type = MYSQL_TYPE_STRING;
        result_bind[7].buffer = status;
        result_bind[7].buffer_length = sizeof(status);
        result_bind[7].length = &status_len;
        result_bind[8].buffer_type = MYSQL_TYPE_LONG;
        result_bind[8].buffer = &progress;
        result_bind[9].buffer_type = MYSQL_TYPE_STRING;
        result_bind[9].buffer = error_message;
        result_bind[9].buffer_length = sizeof(error_message);
        result_bind[9].length = &error_message_len;
        result_bind[10].buffer_type = MYSQL_TYPE_STRING;
        result_bind[10].buffer = created_at;
        result_bind[10].buffer_length = sizeof(created_at);
        result_bind[10].length = &created_at_len;
        result_bind[11].buffer_type = MYSQL_TYPE_STRING;
        result_bind[11].buffer = updated_at;
        result_bind[11].buffer_length = sizeof(updated_at);
        result_bind[11].length = &updated_at_len;
        result_bind[12].buffer_type = MYSQL_TYPE_STRING;
        result_bind[12].buffer = completed_at;
        result_bind[12].buffer_length = sizeof(completed_at);
        result_bind[12].length = &completed_at_len;
        
        if (mysql_stmt_bind_result(stmt, result_bind) == 0) {
            while (mysql_stmt_fetch(stmt) == 0) {
                FFmpegCommand cmd;
                cmd.id = id;
                cmd.userId = user_id;
                cmd.commandName = std::string(command_name, command_name_len);
                cmd.inputFile = std::string(input_file, input_file_len);
                cmd.outputFile = std::string(output_file, output_file_len);
                cmd.commandLine = std::string(command_line, command_line_len);
                cmd.parameters = std::string(parameters, parameters_len);
                cmd.status = std::string(status, status_len);
                cmd.progress = progress;
                cmd.errorMessage = std::string(error_message, error_message_len);
                cmd.createdAt = std::string(created_at, created_at_len);
                cmd.updatedAt = std::string(updated_at, updated_at_len);
                cmd.completedAt = std::string(completed_at, completed_at_len);
                commands.push_back(cmd);
            }
        }
        
        mysql_free_result(result);
    }
    
    mysql_stmt_close(stmt);
    return commands;
}

bool DatabaseManager::getUserStats(int userId, std::map<std::string, int>& stats) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    // 初始化统计数据
    stats["total_commands"] = 0;
    stats["completed_commands"] = 0;
    stats["failed_commands"] = 0;
    stats["pending_commands"] = 0;
    
    // 获取总命令数
    std::string query = "SELECT COUNT(*) FROM ffmpeg_commands WHERE user_id = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &userId;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) == 0) {
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));
        int count;
        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &count;
        
        if (mysql_stmt_bind_result(stmt, result_bind) == 0 && mysql_stmt_fetch(stmt) == 0) {
            stats["total_commands"] = count;
        }
    }
    
    mysql_stmt_close(stmt);
    
    // 获取各状态的命令数
    std::vector<std::string> status_list = {"completed", "failed", "pending"};
    std::vector<std::string> keys = {"completed_commands", "failed_commands", "pending_commands"};
    
    for (size_t i = 0; i < status_list.size(); ++i) {
        stmt = mysql_stmt_init(connection_);
        if (!stmt) continue;
        
        query = "SELECT COUNT(*) FROM ffmpeg_commands WHERE user_id = ? AND status = ?";
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            mysql_stmt_close(stmt);
            continue;
        }
        
        MYSQL_BIND status_bind[2];
        memset(status_bind, 0, sizeof(status_bind));
        status_bind[0].buffer_type = MYSQL_TYPE_LONG;
        status_bind[0].buffer = &userId;
        status_bind[1].buffer_type = MYSQL_TYPE_STRING;
        status_bind[1].buffer = (void*)status_list[i].c_str();
        status_bind[1].buffer_length = status_list[i].length();
        
        if (mysql_stmt_bind_param(stmt, status_bind) == 0 && mysql_stmt_execute(stmt) == 0) {
            MYSQL_BIND status_result[1];
            memset(status_result, 0, sizeof(status_result));
            int status_count;
            status_result[0].buffer_type = MYSQL_TYPE_LONG;
            status_result[0].buffer = &status_count;
            
            if (mysql_stmt_bind_result(stmt, status_result) == 0 && mysql_stmt_fetch(stmt) == 0) {
                stats[keys[i]] = status_count;
            }
        }
        
        mysql_stmt_close(stmt);
    }
    
    return true;
}

bool DatabaseManager::getUserByUsername(const std::string& username, User& user, std::string& storedHash, std::string& salt) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isConnected()) {
        return false;
    }
    
    std::string query = "SELECT id, username, email, role, password_hash, salt, is_active, last_login FROM users WHERE username = ? LIMIT 1";
    
    MYSQL_STMT* stmt = mysql_stmt_init(connection_);
    if (!stmt) {
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)username.c_str();
    bind[0].buffer_length = username.length();
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
    if (result) {
        MYSQL_BIND result_bind[8];
        memset(result_bind, 0, sizeof(result_bind));
        
        int id, is_active;
        char db_username[101], email[101], role[51], password_hash[256], salt_str[256], last_login[20];
        unsigned long username_len, email_len, role_len, password_hash_len, salt_len, last_login_len;
        
        // Bind result columns
        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &id;
        result_bind[1].buffer_type = MYSQL_TYPE_STRING;
        result_bind[1].buffer = db_username;
        result_bind[1].buffer_length = sizeof(db_username);
        result_bind[1].length = &username_len;
        result_bind[2].buffer_type = MYSQL_TYPE_STRING;
        result_bind[2].buffer = email;
        result_bind[2].buffer_length = sizeof(email);
        result_bind[2].length = &email_len;
        result_bind[3].buffer_type = MYSQL_TYPE_STRING;
        result_bind[3].buffer = role;
        result_bind[3].buffer_length = sizeof(role);
        result_bind[3].length = &role_len;
        result_bind[4].buffer_type = MYSQL_TYPE_STRING;
        result_bind[4].buffer = password_hash;
        result_bind[4].buffer_length = sizeof(password_hash);
        result_bind[4].length = &password_hash_len;
        result_bind[5].buffer_type = MYSQL_TYPE_STRING;
        result_bind[5].buffer = salt_str;
        result_bind[5].buffer_length = sizeof(salt_str);
        result_bind[5].length = &salt_len;
        result_bind[6].buffer_type = MYSQL_TYPE_LONG;
        result_bind[6].buffer = &is_active;
        result_bind[7].buffer_type = MYSQL_TYPE_STRING;
        result_bind[7].buffer = last_login;
        result_bind[7].buffer_length = sizeof(last_login);
        result_bind[7].length = &last_login_len;
        
        if (mysql_stmt_bind_result(stmt, result_bind) == 0 && mysql_stmt_fetch(stmt) == 0) {
            user.id = id;
            user.username = std::string(db_username, username_len);
            user.email = std::string(email, email_len);
            user.role = std::string(role, role_len);
            user.isActive = (is_active != 0);
            user.lastLogin = std::string(last_login, last_login_len);
            storedHash = std::string(password_hash, password_hash_len);
            salt = std::string(salt_str, salt_len);
            
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return true;
        }
        
        mysql_free_result(result);
    }
    
    mysql_stmt_close(stmt);
    return false;
}