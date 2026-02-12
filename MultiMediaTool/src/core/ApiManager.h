#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>
#include <memory>

struct User {
    int id;
    QString username;
    QString email;
    QString role;
    bool isActive;
    QString lastLogin;
    
    User() : id(0), isActive(false) {}
};

struct FFmpegCommand {
    int id;
    int userId;
    QString commandName;
    QString inputFile;
    QString outputFile;
    QString commandLine;
    QString parameters; // JSON string
    QString status;
    int progress;
    QString errorMessage;
    QString createdAt;
    QString updatedAt;
    QString completedAt;
    
    FFmpegCommand() : id(0), userId(0), progress(0) {}
};

class ApiManager : public QObject
{
    Q_OBJECT

public:
    explicit ApiManager(QObject *parent = nullptr);
    ~ApiManager();
    
    // 单例模式
    static ApiManager& getInstance();
    
    // 服务器配置
    void setServerUrl(const QString& url);
    QString getServerUrl() const;
    void setTimeout(int milliseconds);
    int getTimeout() const;
    
    // 认证相关
    void login(const QString& username, const QString& password);
    void registerUser(const QString& username, const QString& email, const QString& password);
    void logout();
    void validateToken();
    
    // 用户相关
    void getUserProfile();
    void updateUserProfile(const QJsonObject& userData);
    
    // FFmpeg命令相关
    void saveCommand(const FFmpegCommand& command);
    void updateCommandStatus(int commandId, const QString& status, int progress = 0, const QString& errorMessage = "");
    void getCommand(int commandId);
    void getUserCommands(int limit = 50, int offset = 0);
    void getUserStats();
    
    // 文件相关
    void uploadFile(const QString& filePath, const QString& fileType);
    void downloadFile(const QString& fileId, const QString& savePath);
    
    // 状态获取
    bool isLoggedIn() const;
    QString getCurrentToken() const;
    User getCurrentUser() const;
    
    // 设置管理
    void loadSettings();
    void saveSettings();

signals:
    // 认证信号
    void loginSuccess(const User& user, const QString& token);
    void loginFailed(const QString& error);
    void registerSuccess(const QString& message);
    void registerFailed(const QString& error);
    void logoutSuccess();
    void tokenValidated(const User& user);
    void tokenInvalid(const QString& error);
    
    // 用户相关信号
    void userProfileReceived(const User& user);
    void userProfileUpdateSuccess(const QString& message);
    void userProfileUpdateFailed(const QString& error);
    
    // FFmpeg命令相关信号
    void commandSaved(const FFmpegCommand& command);
    void commandSaveFailed(const QString& error);
    void commandStatusUpdated();
    void commandStatusUpdateFailed(const QString& error);
    void commandReceived(const FFmpegCommand& command);
    void userCommandsReceived(const QList<FFmpegCommand>& commands);
    void userStatsReceived(const QJsonObject& stats);
    
    // 文件相关信号
    void fileUploadSuccess(const QString& fileId);
    void fileUploadFailed(const QString& error);
    void fileUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void fileDownloadSuccess(const QString& filePath);
    void fileDownloadFailed(const QString& error);
    void fileDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    
    // 通用信号
    void networkError(const QString& error);
    void serverConnected();
    void serverDisconnected();

private slots:
    // 网络响应处理
    void handleNetworkReply(QNetworkReply* reply);
    void onTimeout();
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    // 网络管理
    QNetworkAccessManager* networkManager;
    QTimer* timeoutTimer;
    
    // 配置
    QString serverUrl;
    int timeout;
    
    // 认证状态
    QString currentToken;
    User currentUser;
    bool loggedIn;
    
    // 请求管理
    QMap<QNetworkReply*, QString> pendingRequests;
    
    // 私有方法
    QNetworkRequest createRequest(const QString& endpoint, const QByteArray& data = QByteArray(), const QString& method = "GET");
    void sendRequest(const QString& endpoint, const QJsonObject& data = QJsonObject(), const QString& method = "GET", const QString& requestType = "");
    void sendRequestWithAuth(const QString& endpoint, const QJsonObject& data = QJsonObject(), const QString& method = "GET", const QString& requestType = "");
    
    // 响应解析
    void parseResponse(const QByteArray& data, const QString& requestType, QNetworkReply* reply);
    void parseLoginResponse(const QJsonObject& response);
    void parseRegisterResponse(const QJsonObject& response);
    void parseValidateTokenResponse(const QJsonObject& response);
    void parseUserProfileResponse(const QJsonObject& response);
    void parseCommandResponse(const QJsonObject& response);
    void parseCommandsResponse(const QJsonObject& response);
    void parseStatsResponse(const QJsonObject& response);
    
    // 工具方法
    User parseUserFromJson(const QJsonObject& json);
    FFmpegCommand parseCommandFromJson(const QJsonObject& json);
    QJsonObject userToJson(const User& user);
    QJsonObject commandToJson(const FFmpegCommand& command);
    
    // 错误处理
    void handleError(const QString& error, const QString& requestType = "");
    void handleNetworkError(QNetworkReply::NetworkError error, const QString& requestType = "");
    
    // 辅助方法
    QString hashPassword(const QString& password, const QString& salt);
    QString generateSalt();
    void setAuthHeader(QNetworkRequest& request);
    void startTimeoutTimer();
    void stopTimeoutTimer();
};

#endif // APIMANAGER_H