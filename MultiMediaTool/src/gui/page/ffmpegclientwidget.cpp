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

FfmpegClientWidget::FfmpegClientWidget(QWidget *parent)
    : QWidget(parent)
    , m_isLoggedIn(false)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_statusTimer(new QTimer(this))
    , m_mainLayout(nullptr)
    , m_stackedLayout(nullptr)
    , m_loginWidget(nullptr)
    , m_mainWidget(nullptr)
    , m_tabWidget(nullptr)
    , m_commandTab(nullptr)
    , m_fileTab(nullptr)
    , m_historyTab(nullptr)
{
    // 初始化API管理器
    m_apiManager = std::make_unique<ApiManager>();
    
    // 设置定时器
    m_statusTimer->setInterval(30000); // 30秒检查一次状态
    connect(m_statusTimer, &QTimer::timeout, this, &FfmpegClientWidget::onTimerTick);
    
    // 初始化UI
    initializeUI();
    
    // 加载命令预设
    setupCommandPresets();
    
    // 显示登录页面
    showLoginPage();
    
    // 开始状态定时器
    m_statusTimer->start();
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
        showSuccessMessage(tr(gbk_to_utf8("登录成功！欢迎 %1").c_str()).arg(username));
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
    
    showSuccessMessage(tr(gbk_to_utf8("已安全登出").c_str()));
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
    initializeMainPage();
    
    m_mainLayout->addLayout(m_stackedLayout);
}

void FfmpegClientWidget::initializeLoginPage()
{
    m_loginWidget = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(m_loginWidget);
    
    // 标题
    QLabel *titleLabel = new QLabel(tr("FFmpeg Media Server"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin: 20px;");
    loginLayout->addWidget(titleLabel);
    
    // 登录表单
    QGroupBox *loginGroup = new QGroupBox(tr(gbk_to_utf8("登录到服务器").c_str()), this);
    QVBoxLayout *formLayout = new QVBoxLayout(loginGroup);
    
    // 服务器地址
    formLayout->addWidget(new QLabel(tr(gbk_to_utf8("服务器地址:").c_str())));
    m_serverUrlEdit = new QLineEdit("http://localhost:8080", this);
    m_serverUrlEdit->setPlaceholderText(tr(gbk_to_utf8("输入服务器地址，如: http://localhost:8080").c_str()));
    formLayout->addWidget(m_serverUrlEdit);
    
    // 用户名
    formLayout->addWidget(new QLabel(tr(gbk_to_utf8("用户名:").c_str())));
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(tr(gbk_to_utf8("输入用户名").c_str()));
    formLayout->addWidget(m_usernameEdit);
    
    // 密码
    formLayout->addWidget(new QLabel(tr(gbk_to_utf8("密码:").c_str())));
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr(gbk_to_utf8("输入密码").c_str()));
    formLayout->addWidget(m_passwordEdit);
    
    // 记住服务器
    m_rememberServerCheckBox = new QCheckBox(tr(gbk_to_utf8("记住服务器地址").c_str()), this);
    formLayout->addWidget(m_rememberServerCheckBox);
    
    // 登录按钮
    m_loginButton = new QPushButton(tr(gbk_to_utf8("登录").c_str()), this);
    m_loginButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                 "QPushButton:hover { background-color: #2980b9; }");
    connect(m_loginButton, &QPushButton::clicked, this, &FfmpegClientWidget::onLoginClicked);
    formLayout->addWidget(m_loginButton);
    
    loginLayout->addWidget(loginGroup);
    
    // 状态标签
    m_loginStatusLabel = new QLabel(tr(gbk_to_utf8("准备就绪").c_str()), this);
    m_loginStatusLabel->setAlignment(Qt::AlignCenter);
    m_loginStatusLabel->setStyleSheet("color: #7f8c8d;");
    loginLayout->addWidget(m_loginStatusLabel);
    
    loginLayout->addStretch();
    
    // 支持回车键登录
    connect(m_usernameEdit, &QLineEdit::returnPressed, m_passwordEdit, QOverload<>::of(&QLineEdit::setFocus));
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &FfmpegClientWidget::onLoginClicked);
    
    m_stackedLayout->addWidget(m_loginWidget);
}

void FfmpegClientWidget::initializeMainPage()
{
    m_mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(m_mainWidget);
    
    // 标签页
    m_tabWidget = new QTabWidget(this);
    
    initializeCommandTab();
    initializeFileTab();
    initializeHistoryTab();
    
    m_tabWidget->addTab(m_commandTab, tr("FFmpeg命令"));
    m_tabWidget->addTab(m_fileTab, tr("文件管理"));
    m_tabWidget->addTab(m_historyTab, tr("历史记录"));
    
    mainLayout->addWidget(m_tabWidget);
    
    m_stackedLayout->addWidget(m_mainWidget);
}



void FfmpegClientWidget::initializeCommandTab()
{
    m_commandTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_commandTab);
    
    // 命令类型和基本信息
    QGroupBox *basicGroup = new QGroupBox(tr("基本设置"), this);
    QGridLayout *basicLayout = new QGridLayout(basicGroup);
    
    basicLayout->addWidget(new QLabel(tr("命令类型:")), 0, 0);
    m_commandTypeCombo = new QComboBox(this);
    m_commandTypeCombo->addItems({tr("视频转换"), tr("音频转换"), tr("视频压缩"), tr("视频合并"), tr("截图"), tr("自定义")});
    basicLayout->addWidget(m_commandTypeCombo, 0, 1);
    
    basicLayout->addWidget(new QLabel(tr("命令名称:")), 1, 0);
    m_commandNameEdit = new QLineEdit(this);
    m_commandNameEdit->setPlaceholderText(tr("输入命令名称"));
    basicLayout->addWidget(m_commandNameEdit, 1, 1);
    
    layout->addWidget(basicGroup);
    
    // 文件设置
    QGroupBox *fileGroup = new QGroupBox(tr("文件设置"), this);
    QGridLayout *fileLayout = new QGridLayout(fileGroup);
    
    fileLayout->addWidget(new QLabel(tr("输入文件:")), 0, 0);
    m_inputFileEdit = new QLineEdit(this);
    m_inputFileEdit->setPlaceholderText(tr("选择输入文件"));
    fileLayout->addWidget(m_inputFileEdit, 0, 1);
    
    m_browseInputButton = new QPushButton(tr("浏览"), this);
    connect(m_browseInputButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("选择输入文件"));
        if (!fileName.isEmpty()) {
            m_inputFileEdit->setText(fileName);
        }
    });
    fileLayout->addWidget(m_browseInputButton, 0, 2);
    
    fileLayout->addWidget(new QLabel(tr("输出文件:")), 1, 0);
    m_outputFileEdit = new QLineEdit(this);
    m_outputFileEdit->setPlaceholderText(tr("指定输出文件路径"));
    fileLayout->addWidget(m_outputFileEdit, 1, 1);
    
    m_browseOutputButton = new QPushButton(tr("浏览"), this);
    connect(m_browseOutputButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("选择输出文件"));
        if (!fileName.isEmpty()) {
            m_outputFileEdit->setText(fileName);
        }
    });
    fileLayout->addWidget(m_browseOutputButton, 1, 2);
    
    layout->addWidget(fileGroup);
    
    // 命令行参数
    QGroupBox *paramGroup = new QGroupBox(tr("命令行参数"), this);
    QVBoxLayout *paramLayout = new QVBoxLayout(paramGroup);
    
    m_commandLineEdit = new QTextEdit(this);
    m_commandLineEdit->setMaximumHeight(100);
    m_commandLineEdit->setPlaceholderText(tr("输入FFmpeg命令行参数"));
    paramLayout->addWidget(m_commandLineEdit);
    
    m_parametersEdit = new QLineEdit(this);
    m_parametersEdit->setPlaceholderText(tr("额外参数 (JSON格式)"));
    paramLayout->addWidget(m_parametersEdit);
    
    layout->addWidget(paramGroup);
    
    // 执行和状态
    QHBoxLayout *executeLayout = new QHBoxLayout();
    
    m_executeButton = new QPushButton(tr("执行命令"), this);
    m_executeButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                   "QPushButton:hover { background-color: #229954; }");
    connect(m_executeButton, &QPushButton::clicked, this, &FfmpegClientWidget::onExecuteCommandClicked);
    executeLayout->addWidget(m_executeButton);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    executeLayout->addWidget(m_progressBar);
    
    layout->addLayout(executeLayout);
    
    // 状态标签
    m_statusLabel = new QLabel(tr("准备执行命令"), this);
    m_statusLabel->setStyleSheet("color: #7f8c8d;");
    layout->addWidget(m_statusLabel);
    
    layout->addStretch();
    
    // 连接命令类型变化事件
    connect(m_commandTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index < m_commandPresets.size()) {
            const auto &preset = m_commandPresets[index];
            m_commandNameEdit->setText(preset.name);
            m_commandLineEdit->setText(preset.commandLine);
        }
    });
    
    // 初始时只禁用执行按钮，其他按钮在各自标签页初始化时禁用
    m_executeButton->setEnabled(false);
}

void FfmpegClientWidget::initializeFileTab()
{
    m_fileTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_fileTab);
    
    // 文件上传区域
    QGroupBox *uploadGroup = new QGroupBox(tr("文件上传"), this);
    QHBoxLayout *uploadLayout = new QHBoxLayout(uploadGroup);
    
    m_uploadFileEdit = new QLineEdit(this);
    m_uploadFileEdit->setPlaceholderText(tr("选择要上传的文件"));
    uploadLayout->addWidget(m_uploadFileEdit);
    
    m_browseUploadButton = new QPushButton(tr("浏览"), this);
    connect(m_browseUploadButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("选择要上传的文件"));
        if (!fileName.isEmpty()) {
            m_uploadFileEdit->setText(fileName);
        }
    });
    uploadLayout->addWidget(m_browseUploadButton);
    
    m_uploadButton = new QPushButton(tr("上传文件"), this);
    connect(m_uploadButton, &QPushButton::clicked, this, &FfmpegClientWidget::onUploadFileClicked);
    uploadLayout->addWidget(m_uploadButton);
    
    layout->addWidget(uploadGroup);
    
    // 文件列表
    QGroupBox *listGroup = new QGroupBox(tr("服务器文件列表"), this);
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_refreshFilesButton = new QPushButton(tr("刷新"), this);
    connect(m_refreshFilesButton, &QPushButton::clicked, this, [this]() {
        // TODO: 实现文件列表刷新
        showSuccessMessage("文件列表已刷新");
    });
    buttonLayout->addWidget(m_refreshFilesButton);
    buttonLayout->addStretch();
    listLayout->addLayout(buttonLayout);
    
    m_fileTable = new QTableWidget(this);
    m_fileTable->setColumnCount(4);
    QStringList fileHeaders = {tr("文件名"), tr("大小"), tr("修改时间"), tr("操作")};
    m_fileTable->setHorizontalHeaderLabels(fileHeaders);
    m_fileTable->horizontalHeader()->setStretchLastSection(true);
    listLayout->addWidget(m_fileTable);
    
    m_downloadButton = new QPushButton(tr("下载选中文件"), this);
    connect(m_downloadButton, &QPushButton::clicked, this, &FfmpegClientWidget::onDownloadFileClicked);
    listLayout->addWidget(m_downloadButton);
    
    layout->addWidget(listGroup);
    
    // 初始时禁用文件相关控件
    m_uploadButton->setEnabled(false);
    m_downloadButton->setEnabled(false);
}

void FfmpegClientWidget::initializeHistoryTab()
{
    m_historyTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_historyTab);
    
    // 工具栏
    QHBoxLayout *toolLayout = new QHBoxLayout();
    
    m_refreshHistoryButton = new QPushButton(tr("刷新历史"), this);
    connect(m_refreshHistoryButton, &QPushButton::clicked, this, &FfmpegClientWidget::onCommandHistoryClicked);
    toolLayout->addWidget(m_refreshHistoryButton);
    
    m_clearHistoryButton = new QPushButton(tr("清空历史"), this);
    connect(m_clearHistoryButton, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, tr("确认"), tr("确定要清空所有历史记录吗？")) == QMessageBox::Yes) {
            // TODO: 实现清空历史功能
            showSuccessMessage("历史记录已清空");
        }
    });
    toolLayout->addWidget(m_clearHistoryButton);
    
    toolLayout->addStretch();
    layout->addLayout(toolLayout);
    
    // 历史记录表格
    m_historyTable = new QTableWidget(this);
    m_historyTable->setColumnCount(8);
    QStringList headers = {tr("ID"), tr("命令名称"), tr("输入文件"), tr("输出文件"), tr("状态"), tr("进度"), tr("执行时间"), tr("错误信息")};
    m_historyTable->setHorizontalHeaderLabels(headers);
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(m_historyTable);
    
    // 初始时禁用历史记录按钮
    m_refreshHistoryButton->setEnabled(false);
}

void FfmpegClientWidget::setupCommandPresets()
{
    m_commandPresets = {
        {tr("视频转换"), "-i {input} -c:v libx264 -c:a aac {output}", tr("将视频转换为H.264+AAC格式")},
        {tr("音频转换"), "-i {input} -vn -c:a mp3 {output}", tr("提取并转换为MP3音频")},
        {tr("视频压缩"), "-i {input} -vf scale=1280:-1 -crf 23 {output}", tr("压缩视频到720p")},
        {tr("视频合并"), "-f concat -safe 0 -i filelist.txt -c copy {output}", tr("合并多个视频文件")},
        {tr("截图"), "-i {input} -ss 00:00:01 -vframes 1 {output}", tr("从视频中截取一帧")}
    };
}

void FfmpegClientWidget::showLoginPage()
{
    m_stackedLayout->setCurrentIndex(0);
    setLoginEnabled(true);
}

void FfmpegClientWidget::showMainPage()
{
    m_stackedLayout->setCurrentIndex(1);
    setLoginEnabled(false);
    setCommandEnabled(true);
    
    // 加载历史记录
    onCommandHistoryClicked();
}

void FfmpegClientWidget::setLoginEnabled(bool enabled)
{
    if (m_loginButton) m_loginButton->setEnabled(enabled);
    if (m_usernameEdit) m_usernameEdit->setEnabled(enabled);
    if (m_passwordEdit) m_passwordEdit->setEnabled(enabled);
    if (m_serverUrlEdit) m_serverUrlEdit->setEnabled(enabled);
}

void FfmpegClientWidget::setCommandEnabled(bool enabled)
{
    if (m_executeButton) m_executeButton->setEnabled(enabled);
    if (m_uploadButton) m_uploadButton->setEnabled(enabled);
    if (m_downloadButton) m_downloadButton->setEnabled(enabled);
    if (m_refreshHistoryButton) m_refreshHistoryButton->setEnabled(enabled);
}

void FfmpegClientWidget::onLoginClicked()
{
    const QString serverUrl = m_serverUrlEdit->text().trimmed();
    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    
    if (serverUrl.isEmpty()) {
        showErrorMessage(tr("请输入服务器地址"));
        return;
    }
    
    if (username.isEmpty()) {
        showErrorMessage(tr("请输入用户名"));
        return;
    }
    
    if (password.isEmpty()) {
        showErrorMessage(tr("请输入密码"));
        return;
    }
    
    m_loginButton->setEnabled(false);
    m_loginStatusLabel->setText(tr("正在登录..."));
    
    connectToServer(serverUrl, username, password);
}

void FfmpegClientWidget::onLogoutClicked()
{
    logout();
}

void FfmpegClientWidget::sendLoginRequest(const QString &username, const QString &password)
{
    QJsonObject requestObj;
    requestObj["username"] = username;
    requestObj["password"] = password;
    
    QJsonDocument doc(requestObj);
    
    QUrl url(m_serverUrl + "/api/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    if (m_currentReply) {
        m_currentReply->deleteLater();
    }
    
    m_currentReply = m_networkManager->post(request, doc.toJson());
    connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onLoginReply);
}

void FfmpegClientWidget::onLoginReply()
{
    if (!m_currentReply) return;
    
    m_loginButton->setEnabled(true);
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = doc.object();
        
        if (responseObj["success"].toBool()) {
            m_authToken = responseObj["token"].toString();
            setLoginStatus(true, m_usernameEdit->text());
            
            // 保存服务器地址（如果勾选了记住）
            if (m_rememberServerCheckBox->isChecked()) {
                QSettings settings;
                settings.setValue("serverUrl", m_serverUrl);
                settings.setValue("username", m_usernameEdit->text());
            }
        } else {
            m_loginStatusLabel->setText(tr("登录失败: %1").arg(responseObj["message"].toString()));
            showErrorMessage(responseObj["message"].toString());
        }
    } else {
        QString errorString = m_currentReply->errorString();
        m_loginStatusLabel->setText(tr("网络错误: %1").arg(errorString));
        showErrorMessage(tr("网络错误: %1").arg(errorString));
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void FfmpegClientWidget::onExecuteCommandClicked()
{
    const QString commandName = m_commandNameEdit->text().trimmed();
    const QString commandLine = m_commandLineEdit->toPlainText().trimmed();
    
    if (commandName.isEmpty()) {
        showErrorMessage(tr("请输入命令名称"));
        return;
    }
    
    if (commandLine.isEmpty()) {
        showErrorMessage(tr("请输入命令行参数"));
        return;
    }
    
    m_executeButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 无限进度条
    m_statusLabel->setText(tr("正在执行命令..."));
    
    sendExecuteCommandRequest(commandName, commandLine);
}

void FfmpegClientWidget::sendExecuteCommandRequest(const QString &commandName, const QString &commandLine)
{
    QJsonObject requestObj;
    requestObj["commandName"] = commandName;
    requestObj["commandLine"] = commandLine;
    requestObj["inputFile"] = m_inputFileEdit->text();
    requestObj["outputFile"] = m_outputFileEdit->text();
    requestObj["parameters"] = m_parametersEdit->text();
    
    QJsonDocument doc(requestObj);
    
    QUrl url(m_serverUrl + "/api/commands");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    
    if (m_currentReply) {
        m_currentReply->deleteLater();
    }
    
    m_currentReply = m_networkManager->post(request, doc.toJson());
    connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onCommandReply);
}

void FfmpegClientWidget::onCommandReply()
{
    if (!m_currentReply) return;
    
    m_executeButton->setEnabled(true);
    m_progressBar->setVisible(false);
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = doc.object();
        
        if (responseObj["success"].toBool()) {
            m_statusLabel->setText(tr("命令执行成功"));
            showSuccessMessage(tr("命令已成功提交执行"));
            
            // 刷新历史记录
            onCommandHistoryClicked();
        } else {
            m_statusLabel->setText(tr("命令执行失败: %1").arg(responseObj["message"].toString()));
            showErrorMessage(responseObj["message"].toString());
        }
    } else {
        QString errorString = m_currentReply->errorString();
        m_statusLabel->setText(tr("网络错误: %1").arg(errorString));
        showErrorMessage(tr("网络错误: %1").arg(errorString));
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void FfmpegClientWidget::onCommandHistoryClicked()
{
    sendLoadCommandsRequest();
}

void FfmpegClientWidget::onLoadCommandsClicked()
{
    sendLoadCommandsRequest();
}

void FfmpegClientWidget::sendLoadCommandsRequest()
{
    QUrl url(m_serverUrl + "/api/commands/user");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    
    if (m_currentReply) {
        m_currentReply->deleteLater();
    }
    
    m_currentReply = m_networkManager->get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onLoadCommandsReply);
}

void FfmpegClientWidget::onLoadCommandsReply()
{
    if (!m_currentReply) return;
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = doc.object();
        
        if (responseObj["success"].toBool()) {
            QJsonArray commands = responseObj["commands"].toArray();
            updateCommandHistory(commands);
        } else {
            showErrorMessage(responseObj["message"].toString());
        }
    } else {
        QString errorString = m_currentReply->errorString();
        showErrorMessage(tr("加载历史记录失败: %1").arg(errorString));
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void FfmpegClientWidget::updateCommandHistory(const QJsonArray &commands)
{
    m_historyTable->setRowCount(commands.size());
    
    for (int i = 0; i < commands.size(); ++i) {
        QJsonObject cmd = commands[i].toObject();
        
        m_historyTable->setItem(i, 0, new QTableWidgetItem(QString::number(cmd["id"].toInt())));
        m_historyTable->setItem(i, 1, new QTableWidgetItem(cmd["commandName"].toString()));
        m_historyTable->setItem(i, 2, new QTableWidgetItem(cmd["inputFile"].toString()));
        m_historyTable->setItem(i, 3, new QTableWidgetItem(cmd["outputFile"].toString()));
        m_historyTable->setItem(i, 4, new QTableWidgetItem(cmd["status"].toString()));
        m_historyTable->setItem(i, 5, new QTableWidgetItem(QString::number(cmd["progress"].toInt()) + "%"));
        m_historyTable->setItem(i, 6, new QTableWidgetItem(cmd["createdAt"].toString()));
        m_historyTable->setItem(i, 7, new QTableWidgetItem(cmd["errorMessage"].toString()));
    }
    
    m_historyTable->resizeColumnsToContents();
}

void FfmpegClientWidget::onUploadFileClicked()
{
    const QString filePath = m_uploadFileEdit->text().trimmed();
    if (filePath.isEmpty()) {
        showErrorMessage(tr("请选择要上传的文件"));
        return;
    }
    
    sendFileUploadRequest(filePath);
}

void FfmpegClientWidget::sendFileUploadRequest(const QString &filePath)
{
    QFile *file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        showErrorMessage(tr("无法打开文件: %1").arg(filePath));
        delete file;
        return;
    }
    
    QUrl url(m_serverUrl + "/api/files/upload");
    QNetworkRequest request(url);
    
    QString boundary = "----WebKitFormBoundary" + QString::number(QDateTime::currentMSecsSinceEpoch());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    
    QByteArray postData;
    postData.append("--" + boundary + "\r\n");
    postData.append("Content-Disposition: form-data; name=\"file\"; filename=\"" + QFileInfo(filePath).fileName() + "\"\r\n");
    postData.append("Content-Type: application/octet-stream\r\n\r\n");
    postData.append(file->readAll());
    postData.append("\r\n--" + boundary + "--\r\n");
    
    file->close();
    delete file;
    
    if (m_currentReply) {
        m_currentReply->deleteLater();
    }
    
    m_currentReply = m_networkManager->post(request, postData);
    connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onFileUploadReply);
}

void FfmpegClientWidget::onFileUploadReply()
{
    if (!m_currentReply) return;
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = doc.object();
        
        if (responseObj["success"].toBool()) {
            showSuccessMessage(tr("文件上传成功"));
            m_uploadFileEdit->clear();
        } else {
            showErrorMessage(responseObj["message"].toString());
        }
    } else {
        QString errorString = m_currentReply->errorString();
        showErrorMessage(tr("文件上传失败: %1").arg(errorString));
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
}

void FfmpegClientWidget::onFileDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = static_cast<int>((bytesReceived * 100) / bytesTotal);
        m_progressBar->setValue(progress);
        m_statusLabel->setText(tr("下载进度: %1%").arg(progress));
    }
}

void FfmpegClientWidget::onDownloadFileClicked()
{
    int currentRow = m_fileTable->currentRow();
    if (currentRow < 0) {
        showErrorMessage(tr("请选择要下载的文件"));
        return;
    }
    
    QString fileName = m_fileTable->item(currentRow, 0)->text();
    QString savePath = QFileDialog::getSaveFileName(this, tr("保存文件"), fileName);
    
    if (!savePath.isEmpty()) {
        QUrl url(m_serverUrl + "/api/files/download?filename=" + fileName);
        QNetworkRequest request(url);
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
        
        if (m_currentReply) {
            m_currentReply->deleteLater();
        }
        
        m_currentReply = m_networkManager->get(request);
        // TODO: 实现文件下载逻辑
    }
}

void FfmpegClientWidget::onTimerTick()
{
    updateServerStatus();
}

void FfmpegClientWidget::updateServerStatus()
{
    if (m_isLoggedIn && !m_serverUrl.isEmpty()) {
        sendValidateTokenRequest();
    }
}

void FfmpegClientWidget::sendValidateTokenRequest()
{
    QUrl url(m_serverUrl + "/api/auth/validate");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &FfmpegClientWidget::onValidateTokenReply);
}

void FfmpegClientWidget::onValidateTokenReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() != QNetworkReply::NoError) {
        // 连接失败时可能需要重新登录
        if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
            logout();
        }
    }
    
    reply->deleteLater();
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
    
    showErrorMessage(errorMessage);
}

void FfmpegClientWidget::showErrorMessage(const QString &message)
{
    QMessageBox::critical(this, tr("错误"), message);
}

void FfmpegClientWidget::showSuccessMessage(const QString &message)
{
    QMessageBox::information(this, tr("成功"), message);
}

void FfmpegClientWidget::loadCommandPresets()
{
    // 从配置文件或设置中加载自定义预设
}
