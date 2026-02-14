#include "ffmpegclientwidget.h"
#include "src/core/ApiManager.h"
#include "src/utils/lan_util.h"
#include "src/utils/encodinghelper.h"
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QCryptographicHash>
#include <QHeaderView>
#include <QInputDialog>
#include <QSplitter>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QDebug>
#include <QNetworkProxy>
#include <QHttpMultiPart>
#include <QJsonParseError>
#include <QMessageBox>

FfmpegClientWidget::FfmpegClientWidget(QWidget *parent)
    : QWidget(parent)
    , m_isLoggedIn(false)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_statusTimer(new QTimer(this))
    , m_mainLayout(nullptr)
    , m_stackedLayout(nullptr)
    , m_loginWidget(nullptr)
    , m_registerWidget(nullptr)
    , m_mainWidget(nullptr)
{
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 开始初始化").c_str();

    m_apiManager = std::make_unique<ApiManager>();
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] API管理器初始化完成").c_str();

    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] SSL配置完成，验证模式设置为:").c_str() << sslConfig.peerVerifyMode();


    configureNetworkDebugging();


    m_statusTimer->setInterval(30000);
    connect(m_statusTimer, &QTimer::timeout, this, &FfmpegClientWidget::onTimerTick);
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 状态定时器配置完成，间隔:").c_str() << m_statusTimer->interval() << gbk_to_utf8("ms").c_str();


    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 初始化UI").c_str();
    initializeUI();
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] UI初始化完成").c_str();


    showLoginPage();
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录页面显示完成").c_str();


    m_statusTimer->start();
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 状态定时器启动完成").c_str();
}

FfmpegClientWidget::~FfmpegClientWidget()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
}

bool FfmpegClientWidget::isLoggedIn() const
{
    return m_isLoggedIn;
}

void FfmpegClientWidget::setLoginStatus(bool logged, const QString &username)
{
    m_isLoggedIn = logged;
    m_username = username;

    if (logged) {
        showMainPage();
        showSuccessMessage(gbk_to_utf8("登录成功，欢迎 %1").c_str());
        m_statusTimer->start();
    } else {
        showLoginPage();
        m_statusTimer->stop();
    }
}

void FfmpegClientWidget::logout()
{
    setLoginStatus(false);
    m_authToken.clear();
    m_serverUrl.clear();

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    showSuccessMessage(gbk_to_utf8("已安全退出").c_str());
}

bool FfmpegClientWidget::connectToServer(const QString &serverUrl, const QString &username, const QString &password)
{
    m_serverUrl = serverUrl;
    sendLoginRequest(username, password);
    return true;
}

void FfmpegClientWidget::disconnectFromServer()
{
    logout();
}

void FfmpegClientWidget::initializeUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_stackedLayout = new QStackedLayout();

    initializeLoginPage();
    initializeRegisterPage();
    initializeMainPage();

    m_mainLayout->addLayout(m_stackedLayout);
}

void FfmpegClientWidget::initializeLoginPage()
{
    m_loginWidget = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(m_loginWidget);

    // 
    QLabel *titleLabel = new QLabel(gbk_to_utf8("Media Server").c_str(), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin: 20px;");
    loginLayout->addWidget(titleLabel);

    // 
    QGroupBox *loginGroup = new QGroupBox(gbk_to_utf8("登录信息").c_str(), this);
    QVBoxLayout *formLayout = new QVBoxLayout(loginGroup);

    // 
    formLayout->addWidget(new QLabel(gbk_to_utf8("服务器地址:").c_str()));
    m_serverUrlEdit = new QLineEdit("http://localhost:8080", this);
    m_serverUrlEdit->setPlaceholderText(gbk_to_utf8("请输入服务器地址，例如: http://localhost:8080").c_str());
    formLayout->addWidget(m_serverUrlEdit);

    // 
    formLayout->addWidget(new QLabel(gbk_to_utf8("用户名:").c_str()));
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(gbk_to_utf8("请输入用户名").c_str());
    formLayout->addWidget(m_usernameEdit);

    // 
    formLayout->addWidget(new QLabel(gbk_to_utf8("密码:").c_str()));
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(gbk_to_utf8("请输入密码").c_str());
    formLayout->addWidget(m_passwordEdit);

    // 
    m_rememberServerCheckBox = new QCheckBox(gbk_to_utf8("记住服务器地址").c_str(), this);
    formLayout->addWidget(m_rememberServerCheckBox);

    // 
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_loginButton = new QPushButton(gbk_to_utf8("登录").c_str(), this);
    m_loginButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                  "QPushButton:hover { background-color: #2980b9; }");
    connect(m_loginButton, &QPushButton::clicked, this, &FfmpegClientWidget::onLoginClicked);

    m_registerButton = new QPushButton(gbk_to_utf8("注册").c_str(), this);
    m_registerButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                     "QPushButton:hover { background-color: #229954; }");
    connect(m_registerButton, &QPushButton::clicked, this, &FfmpegClientWidget::onShowRegisterPage);

    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_registerButton);
    formLayout->addLayout(buttonLayout);

    loginLayout->addWidget(loginGroup);

    // 
    m_loginStatusLabel = new QLabel(gbk_to_utf8("准备登录").c_str(), this);
    m_loginStatusLabel->setAlignment(Qt::AlignCenter);
    m_loginStatusLabel->setStyleSheet("color: #7f8c8d;");
    loginLayout->addWidget(m_loginStatusLabel);

    loginLayout->addStretch();

    // 
    connect(m_usernameEdit, &QLineEdit::returnPressed, m_passwordEdit, QOverload<>::of(&QLineEdit::setFocus));
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &FfmpegClientWidget::onLoginClicked);

    m_stackedLayout->addWidget(m_loginWidget);
}

void FfmpegClientWidget::initializeRegisterPage()
{
    m_registerWidget = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(m_registerWidget);

    // 
    QLabel *titleLabel = new QLabel(gbk_to_utf8("Media Server - 注册").c_str(), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin: 20px;");
    registerLayout->addWidget(titleLabel);

    // 
    QGroupBox *registerGroup = new QGroupBox(gbk_to_utf8("注册账户").c_str(), this);
    QVBoxLayout *formLayout = new QVBoxLayout(registerGroup);

    // 
    formLayout->addWidget(new QLabel(gbk_to_utf8("服务器地址:").c_str()));
    m_regServerUrlEdit = new QLineEdit("http://localhost:8080", this);
    m_regServerUrlEdit->setPlaceholderText(gbk_to_utf8("请输入服务器地址，例如: http://localhost:8080").c_str());
    formLayout->addWidget(m_regServerUrlEdit);

    // 
    formLayout->addWidget(new QLabel(gbk_to_utf8("用户名:").c_str()));
    m_regUsernameEdit = new QLineEdit(this);
    m_regUsernameEdit->setPlaceholderText(gbk_to_utf8("请输入用户名，至少3个字符").c_str());
    formLayout->addWidget(m_regUsernameEdit);

    // 
    formLayout->addWidget(new QLabel(gbk_to_utf8("邮箱:").c_str()));
    m_regEmailEdit = new QLineEdit(this);
    m_regEmailEdit->setPlaceholderText(gbk_to_utf8("请输入邮箱地址").c_str());
    m_regEmailEdit->setStyleSheet("QLineEdit { font-family: monospace; }");
    formLayout->addWidget(m_regEmailEdit);

    //
    formLayout->addWidget(new QLabel(gbk_to_utf8("密码:").c_str()));
    m_regPasswordEdit = new QLineEdit(this);
    m_regPasswordEdit->setEchoMode(QLineEdit::Password);
    m_regPasswordEdit->setPlaceholderText(gbk_to_utf8("请输入密码，至少6个字符").c_str());
    formLayout->addWidget(m_regPasswordEdit);

    //
    formLayout->addWidget(new QLabel(gbk_to_utf8("确认密码:").c_str()));
    m_regConfirmPasswordEdit = new QLineEdit(this);
    m_regConfirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_regConfirmPasswordEdit->setPlaceholderText(gbk_to_utf8("再次输入密码").c_str());
    formLayout->addWidget(m_regConfirmPasswordEdit);

    // 
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_submitRegisterButton = new QPushButton(gbk_to_utf8("注册").c_str(), this);
    m_submitRegisterButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                          "QPushButton:hover { background-color: #229954; }");
    connect(m_submitRegisterButton, &QPushButton::clicked, this, &FfmpegClientWidget::onRegisterClicked);

    m_backToLoginButton = new QPushButton(gbk_to_utf8("返回登录").c_str(), this);
    m_backToLoginButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                      "QPushButton:hover { background-color: #7f8c8d; }");
    connect(m_backToLoginButton, &QPushButton::clicked, this, &FfmpegClientWidget::onShowLoginPage);

    buttonLayout->addWidget(m_submitRegisterButton);
    buttonLayout->addWidget(m_backToLoginButton);
    formLayout->addLayout(buttonLayout);

    registerLayout->addWidget(registerGroup);

    // 
    m_registerStatusLabel = new QLabel(gbk_to_utf8("准备注册").c_str(), this);
    m_registerStatusLabel->setAlignment(Qt::AlignCenter);
    m_registerStatusLabel->setStyleSheet("color: #7f8c8d;");
    registerLayout->addWidget(m_registerStatusLabel);

    registerLayout->addStretch();

    // 
    connect(m_regUsernameEdit, &QLineEdit::returnPressed, m_regEmailEdit, QOverload<>::of(&QLineEdit::setFocus));
    connect(m_regEmailEdit, &QLineEdit::returnPressed, m_regPasswordEdit, QOverload<>::of(&QLineEdit::setFocus));
    connect(m_regPasswordEdit, &QLineEdit::returnPressed, m_regConfirmPasswordEdit, QOverload<>::of(&QLineEdit::setFocus));
    connect(m_regConfirmPasswordEdit, &QLineEdit::returnPressed, this, &FfmpegClientWidget::onRegisterClicked);

    m_stackedLayout->addWidget(m_registerWidget);
}

void FfmpegClientWidget::initializeMainPage()
{
    m_mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainWidget);

    // 
    QLabel *welcomeLabel = new QLabel(gbk_to_utf8("登录成功，欢迎 %1!").c_str(), this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin: 20px;");
    mainLayout->addWidget(welcomeLabel);

    QLabel *infoLabel = new QLabel(gbk_to_utf8("您已经成功登录到系统，目前系统只提供登录和注册功能。").c_str(), this);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("color: #7f8c8d; margin: 20px;");
    mainLayout->addWidget(infoLabel);

    // �����˳���¼��ť
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *logoutButton = new QPushButton(gbk_to_utf8("退出登录").c_str(), this);
    logoutButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                    "QPushButton:hover { background-color: #c0392b; }");
    connect(logoutButton, &QPushButton::clicked, this, &FfmpegClientWidget::logout);
    buttonLayout->addStretch();
    buttonLayout->addWidget(logoutButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch();

    m_stackedLayout->addWidget(m_mainWidget);
}

void FfmpegClientWidget::onLoginClicked()
{
    QString serverUrl = m_serverUrlEdit->text().trimmed();
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (serverUrl.isEmpty()) {
        showErrorMessage(gbk_to_utf8("请输入服务器地址").c_str());
        return;
    }

    if (username.isEmpty()) {
        showErrorMessage(gbk_to_utf8("请输入用户名").c_str());
        return;
    }

    if (password.isEmpty()) {
        showErrorMessage(gbk_to_utf8("请输入密码").c_str());
        return;
    }

    m_loginButton->setEnabled(false);
    m_loginStatusLabel->setText(gbk_to_utf8("正在登录...").c_str());

    connectToServer(serverUrl, username, password);
}

void FfmpegClientWidget::onLogoutClicked()
{
    logout();
}

void FfmpegClientWidget::onShowRegisterPage()
 {
     // Ԥ���������ַ
     m_regServerUrlEdit->setText(m_serverUrlEdit->text().trimmed());

     // ���ע�����
     m_regUsernameEdit->clear();
     m_regEmailEdit->clear();
     m_regPasswordEdit->clear();
     m_regConfirmPasswordEdit->clear();

     m_registerStatusLabel->setText(gbk_to_utf8("准备注册").c_str());

     // ��ʾע��ҳ��
     m_stackedLayout->setCurrentWidget(m_registerWidget);
 }

 void FfmpegClientWidget::onShowLoginPage()
 {
     // Ԥ���������ַ
     m_serverUrlEdit->setText(m_regServerUrlEdit->text().trimmed());

     m_loginStatusLabel->setText(gbk_to_utf8("准备登录").c_str());

     // ��ʾ��¼ҳ��
     m_stackedLayout->setCurrentWidget(m_loginWidget);
 }

void FfmpegClientWidget::onRegisterClicked()
{
    const QString serverUrl = m_regServerUrlEdit->text().trimmed();
    const QString username = m_regUsernameEdit->text().trimmed();
    const QString email = m_regEmailEdit->text().trimmed();
    const QString password = m_regPasswordEdit->text();
    const QString confirmPassword = m_regConfirmPasswordEdit->text();

    if (serverUrl.isEmpty()) {
        showErrorMessage(gbk_to_utf8("请输入服务器地址").c_str());
        return;
    }

    if (username.isEmpty()) {
        showErrorMessage(gbk_to_utf8("请输入用户名").c_str());
        return;
    }

    if (email.isEmpty()) {
        showErrorMessage(gbk_to_utf8("请输入邮箱").c_str());
        return;
    }

    if (password.isEmpty()) {
        showErrorMessage(gbk_to_utf8("请输入密码").c_str());
        return;
    }

    if (password != confirmPassword) {
        showErrorMessage(gbk_to_utf8("两次密码输入不一致").c_str());
        return;
    }

    m_registerButton->setEnabled(false);
    m_registerStatusLabel->setText(gbk_to_utf8("正在注册...").c_str());

    sendRegisterRequest(serverUrl, username, email, password);
}

void FfmpegClientWidget::onRegisterReply()
{
    if (!m_currentReply) {
        return;
    }

    QByteArray responseData = m_currentReply->readAll();
    int statusCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    m_currentReply->deleteLater();
    m_currentReply = nullptr;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        showErrorMessage(gbk_to_utf8("服务器响应格式错误").c_str());
        setRegisterEnabled(true);
        return;
    }

    QJsonObject responseObj = doc.object();

    if (statusCode >= 200 && statusCode < 300) {
        if (responseObj.contains("success") && responseObj["success"].toBool()) {
            showSuccessMessage(gbk_to_utf8("注册成功，请登录").c_str());
            m_serverUrlEdit->setText(m_regServerUrlEdit->text().trimmed());
            m_usernameEdit->setText(m_regUsernameEdit->text().trimmed());
            m_passwordEdit->clear();
            onShowLoginPage();
        } else {
            QString errorMsg = gbk_to_utf8("注册失败").c_str();
            if (responseObj.contains("message")) {
                errorMsg = responseObj["message"].toString();
            } else if (responseObj.contains("error")) {
                errorMsg = responseObj["error"].toString();
            }
            showErrorMessage(errorMsg.toStdString().c_str());
        }
    } else {
        QString errorMsg = gbk_to_utf8("注册失败").c_str();
        if (responseObj.contains("message")) {
            errorMsg = responseObj["message"].toString();
        } else if (responseObj.contains("error")) {
            errorMsg = responseObj["error"].toString();
        }
        showErrorMessage(errorMsg.toStdString().c_str());
    }

    setRegisterEnabled(true);
}

void FfmpegClientWidget::sendRegisterRequest(const QString &serverUrl, const QString &username,
                                             const QString &email, const QString &password)
{
    QJsonObject requestObj;
    requestObj["username"] = username;
    requestObj["email"] = email;
    requestObj["password"] = password;

    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson();

    QString url = serverUrl.trimmed();
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url;
    }
    if (url.endsWith("/")) {
        url.chop(1);
    }

    QUrl qurl(url + "/api/auth/register");
    QNetworkRequest request(qurl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (m_currentReply) {
        m_currentReply->deleteLater();
    }

    m_currentReply = m_networkManager->post(request, requestData);
    connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onRegisterReply);
}

void FfmpegClientWidget::sendLoginRequest(const QString &username, const QString &password)
{
    // �����Tokenȷ��ʹ����Token
    m_authToken.clear();
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 清除Token，开始新的登录").c_str();

    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 开始登录请求").c_str();
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 服务器URL:").c_str() << m_serverUrl;
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 用户名:").c_str() << username;
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 密码长度:").c_str() << password.length();

    QJsonObject requestObj;
    requestObj["username"] = username;
    requestObj["password"] = password;

    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson();


    QString serverUrl = m_serverUrl.trimmed();
    if (!serverUrl.startsWith("http://") && !serverUrl.startsWith("https://")) {
        serverUrl = "http://" + serverUrl;
        qDebug() << gbk_to_utf8("[FfmpegClientWidget] 自动添加 http:// 前缀:").c_str() << serverUrl;
    }


    if (serverUrl.endsWith("/")) {
        serverUrl.chop(1);
    }

    QUrl url(serverUrl + "/api/auth/login");
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 完整请求URL:").c_str() << url.toString();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    configureNetworkRequest(request);

    logNetworkRequest(request, requestData);

    if (m_currentReply) {
        qDebug() << gbk_to_utf8("[FfmpegClientWidget] 取消之前的请求").c_str();
        m_currentReply->deleteLater();
    }

    m_currentReply = m_networkManager->post(request, requestData);
    connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onLoginReply);
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录请求已发送").c_str();
}

void FfmpegClientWidget::onLoginReply()
{
    if (!m_currentReply) {
        qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录响应为空").c_str();
        return;
    }

    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 开始处理登录响应").c_str();
    logNetworkResponse(m_currentReply);

    m_loginButton->setEnabled(true);

    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录响应数据大小:").c_str() << responseData.size() << gbk_to_utf8("字节").c_str();
        qDebug() << gbk_to_utf8("[FfmpegClientWidget] 响应内容:").c_str() << responseData.left(500);

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << gbk_to_utf8("[FfmpegClientWidget] JSON解析错误:").c_str() << parseError.errorString();
            m_loginStatusLabel->setText(gbk_to_utf8("响应格式错误").c_str());
            showErrorMessage(gbk_to_utf8("服务器响应格式错误").c_str());
        } else {
            QJsonObject responseObj = doc.object();
            qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录响应内容:").c_str() << responseObj;

            if (responseObj["success"].toBool()) {

                QString token = responseObj["token"].toString();
                QString username = responseObj["username"].toString();
                qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录成功，Token长度:").c_str() << token.length();
                qDebug() << gbk_to_utf8("[FfmpegClientWidget] 获取到用户名:").c_str() << username;
                setLoginStatus(true, username);


                if (m_rememberServerCheckBox->isChecked()) {
                    QSettings settings;
                    settings.setValue("serverUrl", m_serverUrl);
                    settings.setValue("username", m_usernameEdit->text());
                    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 已保存登录信息").c_str();
                }
            } else {

                QString errorMsg;
                if (responseObj.contains("data")) {
                    QJsonObject dataObj = responseObj["data"].toObject();
                    errorMsg = dataObj["message"].toString();
                }
                if (errorMsg.isEmpty()) {
                    errorMsg = responseObj["message"].toString();
                }
                qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录失败:").c_str() << errorMsg;
                m_loginStatusLabel->setText(QString("登录失败: %1").arg(errorMsg));
                showErrorMessage(errorMsg.toUtf8().constData());
            }
        }
    } else {
        QString errorString = m_currentReply->errorString();
        int errorCode = m_currentReply->error();
        int statusCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录请求失败:").c_str();
        qDebug() << gbk_to_utf8("[FfmpegClientWidget] 错误代码:").c_str() << errorCode;
        qDebug() << gbk_to_utf8("[FfmpegClientWidget] HTTP状态码:").c_str() << statusCode;
        qDebug() << gbk_to_utf8("[FfmpegClientWidget] 错误信息:").c_str() << errorString;

        m_loginStatusLabel->setText(QString("网络错误: %1").arg(errorString));
        showErrorMessage(QString("网络错误: %1").arg(errorString).toUtf8().constData());
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 登录响应处理完成").c_str();
}

void FfmpegClientWidget::showLoginPage() {
    m_stackedLayout->setCurrentWidget(m_loginWidget);
}

void FfmpegClientWidget::showRegisterPage() {
    m_stackedLayout->setCurrentWidget(m_registerWidget);
}

void FfmpegClientWidget::showMainPage() {
    m_stackedLayout->setCurrentWidget(m_mainWidget);
}

void FfmpegClientWidget::setLoginEnabled(bool enabled) {
    m_loginButton->setEnabled(enabled);
    m_usernameEdit->setEnabled(enabled);
    m_passwordEdit->setEnabled(enabled);
    m_serverUrlEdit->setEnabled(enabled);
}

void FfmpegClientWidget::setRegisterEnabled(bool enabled) {
    m_registerButton->setEnabled(enabled);
    m_regUsernameEdit->setEnabled(enabled);
    m_regPasswordEdit->setEnabled(enabled);
    m_regConfirmPasswordEdit->setEnabled(enabled);
    m_regEmailEdit->setEnabled(enabled);
    m_regServerUrlEdit->setEnabled(enabled);
}

void FfmpegClientWidget::updateServerStatus() {
}

void FfmpegClientWidget::onTimerTick() {
    updateServerStatus();
}

void FfmpegClientWidget::onValidateTokenReply()
 {
     QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
     if (!reply) {
         qDebug() << gbk_to_utf8("[FfmpegClientWidget] Token验证响应为空").c_str();
         return;
     }

     qDebug() << gbk_to_utf8("[FfmpegClientWidget] 开始Token验证响应").c_str();
     logNetworkResponse(reply);

     if (reply->error() != QNetworkReply::NoError) {
         int errorCode = reply->error();
         int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

         qDebug() << gbk_to_utf8("[FfmpegClientWidget] Token验证失败:").c_str();
         qDebug() << gbk_to_utf8("[FfmpegClientWidget] 错误代码:").c_str() << errorCode;
         qDebug() << gbk_to_utf8("[FfmpegClientWidget] HTTP状态码:").c_str() << statusCode;
         qDebug() << gbk_to_utf8("[FfmpegClientWidget] 错误信息:").c_str() << reply->errorString();


         if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
             qDebug() << gbk_to_utf8("[FfmpegClientWidget] Token已过期，需要重新登录").c_str();
             logout();
         }
     } else {
         qDebug() << gbk_to_utf8("[FfmpegClientWidget] Token验证成功").c_str();
         QByteArray responseData = reply->readAll();
         if (!responseData.isEmpty()) {
             qDebug() << gbk_to_utf8("[FfmpegClientWidget] 验证响应:").c_str() << responseData.left(200);
         }
     }

     reply->deleteLater();
     qDebug() << gbk_to_utf8("[FfmpegClientWidget] Token验证响应处理完成").c_str();
}


void FfmpegClientWidget::handleError(const QString &error, QNetworkReply *reply)
{
    QString errorMessage = error;

    if (reply) {
        QByteArray data = reply->readAll();
        if (!data.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("message")) {
                    errorMessage = obj["message"].toString();
                }
            }
        }
    }

    showErrorMessage(errorMessage.toStdString().c_str());
}

void FfmpegClientWidget::showErrorMessage(const char *message)
{
    QMessageBox::critical(this, gbk_to_utf8("错误").c_str(), message);
}

void FfmpegClientWidget::showSuccessMessage(const char *message)
{
    QMessageBox::information(this, gbk_to_utf8("成功").c_str(), message);
}

void FfmpegClientWidget::configureNetworkRequest(QNetworkRequest &request)
{
    // SSL
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);
}

void FfmpegClientWidget::configureNetworkDebugging()
{
    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 配置网络调试").c_str();


    connect(m_networkManager, &QNetworkAccessManager::sslErrors,
            [](QNetworkReply *reply, const QList<QSslError> &errors) {
                qDebug() << gbk_to_utf8("[Network] SSL错误:").c_str();
                for (const QSslError &error : errors) {
                    qDebug() << gbk_to_utf8("[Network]  -").c_str() << error.errorString();
                }
            });


    QNetworkProxy proxy = QNetworkProxy::applicationProxy();
    if (proxy.type() != QNetworkProxy::NoProxy) {
        qDebug() << gbk_to_utf8("[Network] 使用代理:").c_str() << proxy.hostName() << gbk_to_utf8(":").c_str() << proxy.port();
    } else {
        qDebug() << gbk_to_utf8("[Network] 未使用代理").c_str();
    }

    qDebug() << gbk_to_utf8("[FfmpegClientWidget] 网络调试配置完成").c_str();
}

void FfmpegClientWidget::logNetworkRequest(const QNetworkRequest &request, const QByteArray &data)
{
    qDebug() << gbk_to_utf8("[Network] =================== 请求开始 ===================").c_str();
    qDebug() << gbk_to_utf8("[Network] URL:").c_str() << request.url().toString();
    qDebug() << gbk_to_utf8("[Network] 方法:").c_str() << request.attribute(QNetworkRequest::CustomVerbAttribute).toString();


    for (const auto &header : request.rawHeaderList()) {
        qDebug() << gbk_to_utf8("[Network] Header:").c_str() << header << gbk_to_utf8("=").c_str() << request.rawHeader(header);
    }


    if (!data.isEmpty()) {
        QString dataStr = QString::fromUtf8(data.left(1000));
        if (data.size() > 1000) {
            dataStr += "...(truncated)";
        }
        qDebug() << gbk_to_utf8("[Network] Body:").c_str() << dataStr;
        qDebug() << gbk_to_utf8("[Network] Body size:").c_str() << data.size() << gbk_to_utf8("bytes").c_str();
    }

    qDebug() << gbk_to_utf8("[Network] ===============================================").c_str();
}

void FfmpegClientWidget::logNetworkResponse(QNetworkReply *reply)
{
    if (!reply) {
        qDebug() << gbk_to_utf8("[Network] 响应为空").c_str();
        return;
    }

    qDebug() << gbk_to_utf8("[Network] =================== 响应结束 ===================").c_str();
    qDebug() << gbk_to_utf8("[Network] URL:").c_str() << reply->url().toString();
    qDebug() << gbk_to_utf8("[Network] HTTP状态码:").c_str() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << gbk_to_utf8("[Network] 错误代码:").c_str() << reply->error();
    qDebug() << gbk_to_utf8("[Network] 错误信息:").c_str() << reply->errorString();


    for (const auto &header : reply->rawHeaderList()) {
        qDebug() << gbk_to_utf8("[Network] Response Header:").c_str() << header << gbk_to_utf8("=").c_str() << reply->rawHeader(header);
    }

    qDebug() << gbk_to_utf8("[Network] ===============================================").c_str();
}

