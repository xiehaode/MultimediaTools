#include "login.h"
#include "ui_login.h"
#include <QSettings>
#include <QMessageBox>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QUuid>
#include <QApplication>

login::login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::login),
    networkManager(nullptr),
    connectionTimer(nullptr),
    connectionTimeout(10000), // 10秒超时
    loginSuccessful(false)
{
    ui->setupUi(this);
    
    initUI();
    initNetwork();
    loadSettings();
    
    // 检查服务器连接
    QTimer::singleShot(1000, this, &login::checkServerConnection);
}

login::~login()
{
    saveSettings();
    delete connectionTimer;
    delete networkManager;
    delete ui;
}

void login::initUI()
{
    // 设置窗口标题
    setWindowTitle("MultiMediaTool - 登录");
    
    // 设置窗口大小和位置
    setFixedSize(400, 500);
    
    // 设置按钮样式
    ui->loginButton->setEnabled(false);
    ui->registerButton->setEnabled(false);
    
    // 连接信号
    connect(ui->usernameEdit, &QLineEdit::textChanged, this, &login::validateInputs);
    connect(ui->passwordEdit, &QLineEdit::textChanged, this, &login::validateInputs);
    connect(ui->emailEdit, &QLineEdit::textChanged, this, &login::validateInputs);
    
    // 设置默认焦点
    ui->usernameEdit->setFocus();
    
    // 设置工具提示
    ui->usernameEdit->setToolTip("请输入用户名");
    ui->passwordEdit->setToolTip("密码长度至少6位");
    ui->emailEdit->setToolTip("请输入有效的邮箱地址");
    ui->rememberMe->setToolTip("记住登录状态");
    
    // 设置密码显示切换
    ui->showPassword->setCheckable(true);
    
    // 隐藏注册相关字段
    ui->emailEdit->hide();
    ui->emailLabel->hide();
    ui->loginButton->setText("登录");
    ui->registerButton->setText("注册");
}

void login::initNetwork()
{
    networkManager = new QNetworkAccessManager(this);
    connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);
    
    // 连接网络信号
    connect(networkManager, &QNetworkAccessManager::finished,
            this, [this](QNetworkReply* reply) {
                QString requestType = reply->property("requestType").toString();
                
                if (requestType == "login") {
                    handleLoginResponse(reply);
                } else if (requestType == "register") {
                    handleRegisterResponse(reply);
                }
            });
    
    connect(connectionTimer, &QTimer::timeout, this, &login::onConnectionTimeout);
    
    // 配置SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone); // 在生产环境中应该验证证书
    networkManager->setConfiguration(sslConfig);
}

void login::loadSettings()
{
    QSettings settings;
    
    serverUrl = settings.value("server/url", "http://localhost:8080").toString();
    connectionTimeout = settings.value("network/timeout", 10000).toInt();
    
    // 如果记住密码，加载保存的用户名
    if (settings.value("login/rememberMe", false).toBool()) {
        QString savedUsername = settings.value("login/username", "").toString();
        if (!savedUsername.isEmpty()) {
            ui->usernameEdit->setText(savedUsername);
            ui->rememberMe->setChecked(true);
        }
    }
}

void login::saveSettings()
{
    QSettings settings;
    
    settings.setValue("server/url", serverUrl);
    settings.setValue("network/timeout", connectionTimeout);
    
    if (ui->rememberMe->isChecked()) {
        settings.setValue("login/rememberMe", true);
        settings.setValue("login/username", ui->usernameEdit->text());
    } else {
        settings.setValue("login/rememberMe", false);
        settings.remove("login/username");
    }
}

void login::validateInputs()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString email = ui->emailEdit->text().trimmed();
    
    bool isRegisterMode = !ui->emailEdit->isHidden();
    bool usernameValid = !username.isEmpty() && username.length() >= 3;
    bool passwordValid = !password.isEmpty() && password.length() >= 6;
    bool emailValid = !isRegisterMode || (validateEmail(email) && !email.isEmpty());
    
    bool isValid = usernameValid && passwordValid && emailValid;
    
    ui->loginButton->setEnabled(isValid);
    ui->registerButton->setEnabled(isValid);
    
    // 更新状态标签
    if (!usernameValid && !username.isEmpty()) {
        ui->statusLabel->setText("用户名长度至少3位");
    } else if (!passwordValid && !password.isEmpty()) {
        ui->statusLabel->setText("密码长度至少6位");
    } else if (!emailValid && !email.isEmpty() && isRegisterMode) {
        ui->statusLabel->setText("请输入有效的邮箱地址");
    } else {
        ui->statusLabel->setText("");
    }
}

void login::on_loginButton_clicked()
{
    if (ui->loginButton->text() == "登录") {
        attemptLogin();
    } else {
        // 切换回登录模式
        ui->emailEdit->hide();
        ui->emailLabel->hide();
        ui->loginButton->setText("登录");
        ui->registerButton->setText("注册");
        ui->usernameEdit->setFocus();
        clearForm();
    }
}

void login::on_registerButton_clicked()
{
    if (ui->registerButton->text() == "注册") {
        // 切换到注册模式
        ui->emailEdit->show();
        ui->emailLabel->show();
        ui->loginButton->setText("返回登录");
        ui->registerButton->setText("立即注册");
        ui->emailEdit->setFocus();
        clearForm();
    } else {
        attemptRegistration();
    }
}

void login::on_showPassword_toggled(bool checked)
{
    if (checked) {
        ui->passwordEdit->setEchoMode(QLineEdit::Normal);
        ui->showPassword->setText("隐藏密码");
    } else {
        ui->passwordEdit->setEchoMode(QLineEdit::Password);
        ui->showPassword->setText("显示密码");
    }
}

void login::on_rememberMe_toggled(bool checked)
{
    // 记住密码状态的更改会在saveSettings时保存
    Q_UNUSED(checked)
}

void login::attemptLogin()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        showMessage("请填写完整的登录信息", true);
        return;
    }
    
    setLoadingState(true);
    sendLoginRequest(username, password);
}

void login::attemptRegistration()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    
    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        showMessage("请填写完整的注册信息", true);
        return;
    }
    
    if (!validateEmail(email)) {
        showMessage("请输入有效的邮箱地址", true);
        return;
    }
    
    setLoadingState(true);
    sendRegisterRequest(username, email, password);
}

void login::sendLoginRequest(const QString& username, const QString& password)
{
    QJsonObject loginData;
    loginData["username"] = username;
    
    // 密码哈希处理
    QString hashedPassword = hashPassword(password, username); // 使用用户名作为盐
    loginData["password"] = hashedPassword;
    
    QJsonDocument doc(loginData);
    QByteArray data = doc.toJson();
    
    QUrl url(serverUrl + "/api/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "MultiMediaTool/1.0");
    
    request.setProperty("requestType", "login");
    
    connectionTimer->start(connectionTimeout);
    networkManager->post(request, data);
}

void login::sendRegisterRequest(const QString& username, const QString& email, const QString& password)
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
    
    QJsonDocument doc(registerData);
    QByteArray data = doc.toJson();
    
    QUrl url(serverUrl + "/api/auth/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "MultiMediaTool/1.0");
    
    request.setProperty("requestType", "register");
    
    connectionTimer->start(connectionTimeout);
    networkManager->post(request, data);
}

void login::handleLoginResponse(QNetworkReply* reply)
{
    connectionTimer->stop();
    setLoadingState(false);
    
    if (reply->error() != QNetworkReply::NoError) {
        handleNetworkError(reply->error());
        showMessage("网络错误: " + reply->errorString(), true);
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        showMessage("服务器响应格式错误", true);
        reply->deleteLater();
        return;
    }
    
    QJsonObject response = jsonDoc.object();
    bool success = response.value("success").toBool();
    
    if (success) {
        QJsonObject data = response.value("data").toObject();
        currentUser = data.value("token").toString();
        currentToken = data.value("token").toString();
        
        QJsonObject user = data.value("user").toObject();
        currentRole = user.value("role").toString();
        
        loginSuccessful = true;
        showMessage("登录成功！");
        
        // 发送登录成功信号
        emit loginSuccess(ui->usernameEdit->text(), currentToken, currentRole);
    } else {
        QString error = response.value("error").toString();
        showMessage("登录失败: " + error, true);
        emit loginFailed(error);
    }
    
    reply->deleteLater();
}

void login::handleRegisterResponse(QNetworkReply* reply)
{
    connectionTimer->stop();
    setLoadingState(false);
    
    if (reply->error() != QNetworkReply::NoError) {
        handleNetworkError(reply->error());
        showMessage("网络错误: " + reply->errorString(), true);
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        showMessage("服务器响应格式错误", true);
        reply->deleteLater();
        return;
    }
    
    QJsonObject response = jsonDoc.object();
    bool success = response.value("success").toBool();
    
    if (success) {
        showMessage("注册成功！请登录");
        
        // 切换回登录模式
        on_loginButton_clicked();
    } else {
        QString error = response.value("error").toString();
        showMessage("注册失败: " + error, true);
    }
    
    reply->deleteLater();
}

void login::checkServerConnection()
{
    QUrl url(serverUrl + "/api/auth/validate");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("User-Agent", "MultiMediaTool/1.0");
    
    // 发送一个空请求检查服务器状态
    QNetworkReply* reply = networkManager->get(request);
    
    // 设置超时
    QTimer::singleShot(5000, [reply]() {
        if (reply->isRunning()) {
            reply->abort();
        }
    });
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() == QNetworkReply::ContentNotFoundError) {
                // 404是正常的，因为我们要的是服务器响应
                ui->statusLabel->setText("服务器连接正常");
            } else {
                ui->statusLabel->setText("服务器连接失败: " + reply->errorString());
            }
        } else {
            ui->statusLabel->setText("服务器连接正常");
        }
        reply->deleteLater();
    });
}

void login::onConnectionTimeout()
{
    setLoadingState(false);
    showMessage("连接超时，请检查网络连接", true);
}

QString login::hashPassword(const QString& password, const QString& salt)
{
    QString combined = password + salt;
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

QString login::generateSalt()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool login::validateEmail(const QString& email)
{
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex.match(email).hasMatch();
}

bool login::validatePassword(const QString& password)
{
    return password.length() >= 6;
}

void login::showMessage(const QString& message, bool isError)
{
    ui->statusLabel->setText(message);
    if (isError) {
        ui->statusLabel->setStyleSheet("color: red;");
    } else {
        ui->statusLabel->setStyleSheet("color: green;");
    }
    
    // 5秒后清除消息
    QTimer::singleShot(5000, [this]() {
        ui->statusLabel->setText("");
        ui->statusLabel->setStyleSheet("");
    });
}

void login::clearForm()
{
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
    ui->emailEdit->clear();
    ui->statusLabel->setText("");
    ui->statusLabel->setStyleSheet("");
}

void login::setLoadingState(bool loading)
{
    ui->usernameEdit->setEnabled(!loading);
    ui->passwordEdit->setEnabled(!loading);
    ui->emailEdit->setEnabled(!loading);
    ui->loginButton->setEnabled(!loading);
    ui->registerButton->setEnabled(!loading);
    ui->showPassword->setEnabled(!loading);
    ui->rememberMe->setEnabled(!loading);
    
    if (loading) {
        ui->loginButton->setText("处理中...");
        ui->registerButton->setText("处理中...");
    } else {
        ui->loginButton->setText(ui->emailEdit->isHidden() ? "登录" : "返回登录");
        ui->registerButton->setText(ui->emailEdit->isHidden() ? "注册" : "立即注册");
    }
}

void login::handleNetworkError(QNetworkReply::NetworkError error)
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
    case QNetworkReply::UnknownServerError:
        errorMessage = "服务器错误";
        break;
    default:
        errorMessage = "网络错误";
        break;
    }
    
    showMessage(errorMessage, true);
}

QString login::getUsername() const
{
    return ui->usernameEdit->text().trimmed();
}

QString login::getToken() const
{
    return currentToken;
}

QString login::getRole() const
{
    return currentRole;
}

bool login::isLoggedIn() const
{
    return loginSuccessful && !currentToken.isEmpty();
}