#include "ApiManager.h"
#include <QApplication>
#include <QSettings>
#include <QJsonParseError>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QUuid>
#include <QDebug>

ApiManager::ApiManager(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
    , timeoutTimer(new QTimer(this))
    , timeout(10000)
    , loggedIn(false)
{
    // 初始化网络管理器
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &ApiManager::handleNetworkReply);
    
    // 设置超时定时器
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &ApiManager::onTimeout);
    
    // 加载设置
    loadSettings();
    
    // 配置SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    networkManager->setConfiguration(sslConfig);
}

ApiManager::~ApiManager()
{
    saveSettings();
}

ApiManager& ApiManager::getInstance()
{
    static ApiManager instance;
    return instance;
}

void ApiManager::setServerUrl(const QString& url)
{
    serverUrl = url;
    emit serverConnected();
}

QString ApiManager::getServerUrl() const
{
    return serverUrl;
}

void ApiManager::setTimeout(int milliseconds)
{
    timeout = milliseconds;
}

int ApiManager::getTimeout() const
{
    return timeout;
}

void ApiManager::login(const QString& username, const QString& password)
{
    QJsonObject loginData;
    loginData["username"] = username;
    
    // 密码哈希处理
    QString hashedPassword = hashPassword(password, username);
    loginData["password"] = hashedPassword;
    
    sendRequest("/api/auth/login", loginData, "POST", "login");
}

void ApiManager::registerUser(const QString& username, const QString& email, const QString& password)
{
    QJsonObject registerData;
    registerData["username"] = username;
    registerData["email"] = email;
    
    // 密码哈希处理
    QString salt = generateSalt();
    QString hashedPassword = hashPassword(password, salt);
    registerData["password"] = hashedPassword;
    registerData["salt"] = salt;
    registerData["role"] = "user";
    
    sendRequest("/api/auth/register", registerData, "POST", "register");
}

void ApiManager::logout()
{
    if (!loggedIn) {
        emit logoutSuccess();
        return;
    }
    
    sendRequestWithAuth("/api/auth/logout", QJsonObject(), "POST", "logout");
}

void ApiManager::validateToken()
{
    if (currentToken.isEmpty()) {
        emit tokenInvalid("No token available");
        return;
    }
    
    sendRequestWithAuth("/api/auth/validate", QJsonObject(), "GET", "validateToken");
}

void ApiManager::getUserProfile()
{
    sendRequestWithAuth("/api/user/profile", QJsonObject(), "GET", "getUserProfile");
}

void ApiManager::updateUserProfile(const QJsonObject& userData)
{
    sendRequestWithAuth("/api/user/profile", userData, "PUT", "updateUserProfile");
}

void ApiManager::saveCommand(const FFmpegCommand& command)
{
    QJsonObject commandData = commandToJson(command);
    sendRequestWithAuth("/api/commands", commandData, "POST", "saveCommand");
}

void ApiManager::updateCommandStatus(int commandId, const QString& status, int progress, const QString& errorMessage)
{
    QJsonObject updateData;
    updateData["commandId"] = commandId;
    updateData["status"] = status;
    updateData["progress"] = progress;
    if (!errorMessage.isEmpty()) {
        updateData["errorMessage"] = errorMessage;
    }
    
    sendRequestWithAuth("/api/commands", updateData, "PUT", "updateCommandStatus");
}

void ApiManager::getCommand(int commandId)
{
    QString endpoint = QString("/api/commands?id=%1").arg(commandId);
    sendRequestWithAuth(endpoint, QJsonObject(), "GET", "getCommand");
}

void ApiManager::getUserCommands(int limit, int offset)
{
    QString endpoint = QString("/api/commands/user?limit=%1&offset=%2").arg(limit).arg(offset);
    sendRequestWithAuth(endpoint, QJsonObject(), "GET", "getUserCommands");
}

void ApiManager::getUserStats()
{
    sendRequestWithAuth("/api/commands/stats", QJsonObject(), "GET", "getUserStats");
}

void ApiManager::uploadFile(const QString& filePath, const QString& fileType)
{
    if (!loggedIn) {
        emit fileUploadFailed("Not logged in");
        return;
    }
    
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit fileUploadFailed("Cannot open file: " + filePath);
        delete file;
        return;
    }
    
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    // 添加文件
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QString("form-data; name=\"file\"; filename=\"%1\"").arg(QFileInfo(filePath).fileName()));
    filePart.setBodyDevice(file);
    file->setParent(multiPart); // 确保文件随multiPart一起删除
    
    multiPart->append(filePart);
    
    // 添加文件类型
    QHttpPart typePart;
    typePart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"type\"");
    typePart.setBody(fileType.toUtf8());
    multiPart->append(typePart);
    
    QString endpoint = "/api/files/upload";
    QNetworkRequest request = createRequest(endpoint);
    setAuthHeader(request);
    
    QNetworkReply* reply = networkManager->post(request, multiPart);
    pendingRequests[reply] = "uploadFile";
    
    // 连接进度信号
    connect(reply, &QNetworkReply::uploadProgress, this, &ApiManager::onUploadProgress);
    
    startTimeoutTimer();
}

void ApiManager::downloadFile(const QString& fileId, const QString& savePath)
{
    if (!loggedIn) {
        emit fileDownloadFailed("Not logged in");
        return;
    }
    
    QString endpoint = QString("/api/files/download?id=%1").arg(fileId);
    QNetworkRequest request = createRequest(endpoint);
    setAuthHeader(request);
    
    QNetworkReply* reply = networkManager->get(request);
    pendingRequests[reply] = "downloadFile";
    
    // 连接进度信号
    connect(reply, &QNetworkReply::downloadProgress, this, &ApiManager::onDownloadProgress);
    
    startTimeoutTimer();
}

bool ApiManager::isLoggedIn() const
{
    return loggedIn && !currentToken.isEmpty();
}

QString ApiManager::getCurrentToken() const
{
    return currentToken;
}

User ApiManager::getCurrentUser() const
{
    return currentUser;
}

void ApiManager::loadSettings()
{
    QSettings settings;
    
    serverUrl = settings.value("server/url", "http://localhost:8080").toString();
    timeout = settings.value("network/timeout", 10000).toInt();
    currentToken = settings.value("auth/token", "").toString();
    
    if (!currentToken.isEmpty()) {
        // 自动验证token
        validateToken();
    }
}

void ApiManager::saveSettings()
{
    QSettings settings;
    
    settings.setValue("server/url", serverUrl);
    settings.setValue("network/timeout", timeout);
    
    if (isLoggedIn()) {
        settings.setValue("auth/token", currentToken);
        settings.setValue("auth/userId", currentUser.id);
        settings.setValue("auth/username", currentUser.username);
    } else {
        settings.remove("auth/token");
        settings.remove("auth/userId");
        settings.remove("auth/username");
    }
}

QNetworkRequest ApiManager::createRequest(const QString& endpoint, const QByteArray& data, const QString& method)
{
    QUrl url(serverUrl + endpoint);
    QNetworkRequest request(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "MultiMediaTool/1.0");
    
    return request;
}

void ApiManager::sendRequest(const QString& endpoint, const QJsonObject& data, const QString& method, const QString& requestType)
{
    QJsonDocument doc(data);
    QByteArray jsonData = doc.toJson();
    
    QNetworkRequest request = createRequest(endpoint, jsonData, method);
    
    QNetworkReply* reply = nullptr;
    
    if (method == "GET") {
        reply = networkManager->get(request);
    } else if (method == "POST") {
        reply = networkManager->post(request, jsonData);
    } else if (method == "PUT") {
        reply = networkManager->put(request, jsonData);
    } else if (method == "DELETE") {
        reply = networkManager->deleteResource(request);
    }
    
    if (reply) {
        pendingRequests[reply] = requestType;
        startTimeoutTimer();
    }
}

void ApiManager::sendRequestWithAuth(const QString& endpoint, const QJsonObject& data, const QString& method, const QString& requestType)
{
    QJsonDocument doc(data);
    QByteArray jsonData = doc.toJson();
    
    QNetworkRequest request = createRequest(endpoint, jsonData, method);
    setAuthHeader(request);
    
    QNetworkReply* reply = nullptr;
    
    if (method == "GET") {
        reply = networkManager->get(request);
    } else if (method == "POST") {
        reply = networkManager->post(request, jsonData);
    } else if (method == "PUT") {
        reply = networkManager->put(request, jsonData);
    } else if (method == "DELETE") {
        reply = networkManager->deleteResource(request);
    }
    
    if (reply) {
        pendingRequests[reply] = requestType;
        startTimeoutTimer();
    }
}

void ApiManager::handleNetworkReply(QNetworkReply* reply)
{
    QString requestType = pendingRequests.take(reply);
    
    if (requestType.isEmpty()) {
        reply->deleteLater();
        return;
    }
    
    stopTimeoutTimer();
    
    if (reply->error() != QNetworkReply::NoError) {
        handleNetworkError(reply->error(), requestType);
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        handleError("JSON parse error: " + parseError.errorString(), requestType);
        reply->deleteLater();
        return;
    }
    
    parseResponse(responseData, requestType, reply);
    reply->deleteLater();
}

void ApiManager::parseResponse(const QByteArray& data, const QString& requestType, QNetworkReply* reply)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    QJsonObject response = jsonDoc.object();
    
    bool success = response.value("success").toBool();
    QJsonObject dataObj = response.value("data").toObject();
    QString error = response.value("error").toString();
    
    if (!success && !error.isEmpty()) {
        handleError(error, requestType);
        return;
    }
    
    // 根据请求类型处理响应
    if (requestType == "login") {
        parseLoginResponse(response);
    } else if (requestType == "register") {
        parseRegisterResponse(response);
    } else if (requestType == "validateToken") {
        parseValidateTokenResponse(response);
    } else if (requestType == "getUserProfile") {
        parseUserProfileResponse(response);
    } else if (requestType == "saveCommand") {
        parseCommandResponse(response);
    } else if (requestType == "getUserCommands") {
        parseCommandsResponse(response);
    } else if (requestType == "getUserStats") {
        parseStatsResponse(response);
    } else if (requestType == "logout") {
        loggedIn = false;
        currentToken.clear();
        currentUser = User();
        saveSettings();
        emit logoutSuccess();
    } else if (requestType == "updateUserProfile") {
        emit userProfileUpdateSuccess("Profile updated successfully");
    } else if (requestType == "updateCommandStatus") {
        emit commandStatusUpdated();
    }
}

void ApiManager::parseLoginResponse(const QJsonObject& response)
{
    QJsonObject data = response.value("data").toObject();
    currentToken = data.value("token").toString();
    currentUser = parseUserFromJson(data.value("user").toObject());
    loggedIn = true;
    
    saveSettings();
    emit loginSuccess(currentUser, currentToken);
}

void ApiManager::parseRegisterResponse(const QJsonObject& response)
{
    QJsonObject data = response.value("data").toObject();
    QString username = data.value("username").toString();
    
    emit registerSuccess("User " + username + " registered successfully");
}

void ApiManager::parseValidateTokenResponse(const QJsonObject& response)
{
    QJsonObject data = response.value("data").toObject();
    User user = parseUserFromJson(data.value("user").toObject());
    
    currentUser = user;
    loggedIn = true;
    
    emit tokenValidated(user);
}

void ApiManager::parseUserProfileResponse(const QJsonObject& response)
{
    QJsonObject data = response.value("data").toObject();
    User user = parseUserFromJson(data);
    
    emit userProfileReceived(user);
}

void ApiManager::parseCommandResponse(const QJsonObject& response)
{
    QJsonObject data = response.value("data").toObject();
    FFmpegCommand command = parseCommandFromJson(data);
    
    emit commandSaved(command);
}

void ApiManager::parseCommandsResponse(const QJsonObject& response)
{
    QJsonArray commandsArray = response.value("data").toArray();
    QList<FFmpegCommand> commands;
    
    for (const auto& item : commandsArray) {
        FFmpegCommand command = parseCommandFromJson(item.toObject());
        commands.append(command);
    }
    
    emit userCommandsReceived(commands);
}

void ApiManager::parseStatsResponse(const QJsonObject& response)
{
    QJsonObject stats = response.value("data").toObject();
    emit userStatsReceived(stats);
}

User ApiManager::parseUserFromJson(const QJsonObject& json)
{
    User user;
    user.id = json.value("id").toInt();
    user.username = json.value("username").toString();
    user.email = json.value("email").toString();
    user.role = json.value("role").toString();
    user.isActive = json.value("isActive").toBool();
    user.lastLogin = json.value("lastLogin").toString();
    return user;
}

FFmpegCommand ApiManager::parseCommandFromJson(const QJsonObject& json)
{
    FFmpegCommand command;
    command.id = json.value("id").toInt();
    command.userId = json.value("userId").toInt();
    command.commandName = json.value("commandName").toString();
    command.inputFile = json.value("inputFile").toString();
    command.outputFile = json.value("outputFile").toString();
    command.commandLine = json.value("commandLine").toString();
    command.parameters = json.value("parameters").toString();
    command.status = json.value("status").toString();
    command.progress = json.value("progress").toInt();
    command.errorMessage = json.value("errorMessage").toString();
    command.createdAt = json.value("createdAt").toString();
    command.updatedAt = json.value("updatedAt").toString();
    command.completedAt = json.value("completedAt").toString();
    return command;
}

QJsonObject ApiManager::userToJson(const User& user)
{
    QJsonObject json;
    json["id"] = user.id;
    json["username"] = user.username;
    json["email"] = user.email;
    json["role"] = user.role;
    json["isActive"] = user.isActive;
    json["lastLogin"] = user.lastLogin;
    return json;
}

QJsonObject ApiManager::commandToJson(const FFmpegCommand& command)
{
    QJsonObject json;
    json["commandName"] = command.commandName;
    json["inputFile"] = command.inputFile;
    json["outputFile"] = command.outputFile;
    json["commandLine"] = command.commandLine;
    json["parameters"] = command.parameters;
    return json;
}

QString ApiManager::hashPassword(const QString& password, const QString& salt)
{
    QString combined = password + salt;
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

QString ApiManager::generateSalt()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void ApiManager::setAuthHeader(QNetworkRequest& request)
{
    request.setRawHeader("Authorization", "Bearer " + currentToken.toUtf8());
}

void ApiManager::startTimeoutTimer()
{
    timeoutTimer->start(timeout);
}

void ApiManager::stopTimeoutTimer()
{
    if (timeoutTimer->isActive()) {
        timeoutTimer->stop();
    }
}

void ApiManager::handleError(const QString& error, const QString& requestType)
{
    qDebug() << "API Error:" << error << "Request:" << requestType;
    emit networkError(error);
}

void ApiManager::handleNetworkError(QNetworkReply::NetworkError error, const QString& requestType)
{
    QString errorMessage;
    switch (error) {
    case QNetworkReply::ConnectionRefusedError:
        errorMessage = "服务器拒绝连接";
        break;
    case QNetworkReply::TimeoutError:
        errorMessage = "连接超时";
        break;
    case QNetworkReply::HostNotFoundError:
        errorMessage = "服务器地址未找到";
        break;
    case QNetworkReply::AuthenticationRequiredError:
        errorMessage = "需要认证";
        break;
    case QNetworkReply::ContentNotFoundError:
        errorMessage = "请求的资源不存在";
        break;
    default:
        errorMessage = "网络错误";
        break;
    }
    
    qDebug() << "Network Error:" << errorMessage << "Code:" << error << "Request:" << requestType;
    emit networkError(errorMessage);
}

void ApiManager::onTimeout()
{
    // 取消所有未完成的请求
    for (auto it = pendingRequests.begin(); it != pendingRequests.end(); ++it) {
        QNetworkReply* reply = it.key();
        QString requestType = it.value();
        
        reply->abort();
        handleError("请求超时", requestType);
    }
    
    pendingRequests.clear();
}

void ApiManager::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        emit fileUploadProgress(bytesSent, bytesTotal);
    }
}

void ApiManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        emit fileDownloadProgress(bytesReceived, bytesTotal);
    }
}