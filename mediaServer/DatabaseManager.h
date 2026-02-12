#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <mysql/mysql.h>
#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <map>

struct User {
    int id;
    std::string username;
    std::string email;
    std::string role;
    bool isActive;
    std::string lastLogin;
};

struct FFmpegCommand {
    int id;
    int userId;
    std::string commandName;
    std::string inputFile;
    std::string outputFile;
    std::string commandLine;
    std::string parameters; // JSON string
    std::string status;
    int progress;
    std::string errorMessage;
    std::string createdAt;
    std::string updatedAt;
    std::string completedAt;
};

class DatabaseManager {
public:
    static DatabaseManager& getInstance();
    
    // 连接管理
    bool connect(const std::string& host, const std::string& user, 
                 const std::string& password, const std::string& database, int port = 3306);
    void disconnect();
    bool isConnected() const;
    
    // 用户管理
    bool registerUser(const std::string& username, const std::string& email,
                      const std::string& passwordHash, const std::string& salt,
                      const std::string& role, int& userId, std::string& message);
    
    // 重载版本，用于不需要返回userId和message的情况
    bool registerUser(const std::string& username, const std::string& email,
                      const std::string& passwordHash, const std::string& salt,
                      const std::string& role = "user");
    
    bool authenticateUser(const std::string& username, const std::string& passwordHash,
                        User& user, std::string& message);
    
    bool getUserById(int userId, User& user);
    
    // 会话管理
    bool createSession(int userId, const std::string& sessionToken,
                      const std::string& ipAddress, const std::string& userAgent,
                      int& sessionId);
    
    bool validateSession(const std::string& sessionToken, User& user);
    
    void cleanupExpiredSessions();
    
    // FFmpeg命令管理
    bool saveFFmpegCommand(const FFmpegCommand& command, int& commandId);
    bool updateCommandStatus(int commandId, const std::string& status, 
                            int progress, const std::string& errorMessage);
    
    bool getCommandById(int commandId, FFmpegCommand& command);
    std::vector<FFmpegCommand> getUserCommands(int userId, int limit = 50, int offset = 0);
    
    // 日志管理
    bool logLoginAttempt(const std::string& username, const std::string& ipAddress,
                        const std::string& userAgent, bool success,
                        const std::string& failureReason = "");
    
    // 获取用户统计信息
    bool getUserStats(int userId, std::map<std::string, int>& stats);
    
private:
    DatabaseManager();
    ~DatabaseManager();
    
    // 禁用拷贝构造和赋值
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    // MySQL连接
    MYSQL* connection_;
    std::mutex mutex_;
    bool connected_;
    
    // 内部辅助函数
    bool executeProcedure(const std::string& procedure, 
                          const std::vector<std::string>& params,
                          std::vector<std::vector<std::string>>& results);
    
    bool executePreparedQuery(const std::string& query,
                             const std::vector<std::string>& params,
                             std::vector<std::vector<std::string>>& results);
    
    User parseUserFromRow(const std::vector<std::string>& row);
    FFmpegCommand parseCommandFromRow(const std::vector<std::string>& row);
    
    // 错误处理
    std::string getMySQLError();
    void logError(const std::string& operation, const std::string& error);
};

#endif // DATABASEMANAGER_H