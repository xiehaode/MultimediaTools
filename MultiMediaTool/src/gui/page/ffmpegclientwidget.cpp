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
    , m_tabWidget(nullptr)
    , m_commandTab(nullptr)
    , m_fileTab(nullptr)
    , m_historyTab(nullptr)
{
    qDebug() << "[FfmpegClientWidget] 开始初始化";
    
    // 初始化API管理器
    m_apiManager = std::make_unique<ApiManager>();
    qDebug() << "[FfmpegClientWidget] API管理器初始化完成";
    
    // 配置SSL设置，用于网络请求
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    qDebug() << "[FfmpegClientWidget] SSL配置完成，验证模式设置为:" << sslConfig.peerVerifyMode();
    
    // 配置网络调试
    configureNetworkDebugging();
    
    // 设置定时器
    m_statusTimer->setInterval(30000); // 30秒检查一次状态
    connect(m_statusTimer, &QTimer::timeout, this, &FfmpegClientWidget::onTimerTick);
    qDebug() << "[FfmpegClientWidget] 状态定时器配置完成，间隔:" << m_statusTimer->interval() << "ms";
    
    // 初始化UI
    qDebug() << "[FfmpegClientWidget] 开始初始化UI";
    initializeUI();
    qDebug() << "[FfmpegClientWidget] UI初始化完成";
    
    // 设置命令预设
    setupCommandPresets();
    qDebug() << "[FfmpegClientWidget] 命令预设设置完成";
    
    // 显示登录页面
    showLoginPage();
    qDebug() << "[FfmpegClientWidget] 登录页面显示完成";
    
    // 启动状态定时器
    m_statusTimer->start();
    qDebug() << "[FfmpegClientWidget] 状态定时器启动完成，构造函数初始化完成";
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
        showSuccessMessage(tr("登录成功，欢迎 %1").arg(username));
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
    
    showSuccessMessage(tr("已安全退出"));
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
    
    // ����
    QLabel *titleLabel = new QLabel(tr("FFmpeg Media Server"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin: 20px;");
    loginLayout->addWidget(titleLabel);
    
    // 登录表单
    QGroupBox *loginGroup = new QGroupBox(tr("登录信息"), this);
    QVBoxLayout *formLayout = new QVBoxLayout(loginGroup);
    
    // 服务器地址
    formLayout->addWidget(new QLabel(tr("服务器地址:")));
    m_serverUrlEdit = new QLineEdit("http://localhost:8080", this);
    m_serverUrlEdit->setPlaceholderText(tr("请输入服务器地址，例如: http://localhost:8080"));
    formLayout->addWidget(m_serverUrlEdit);
    
    // 用户名
    formLayout->addWidget(new QLabel(tr("用户名:")));
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(tr("请输入用户名"));
    formLayout->addWidget(m_usernameEdit);
    
    // 密码
    formLayout->addWidget(new QLabel(tr("密码:")));
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("请输入密码"));
    formLayout->addWidget(m_passwordEdit);
    
    // 记住服务器
    m_rememberServerCheckBox = new QCheckBox(tr("记住服务器地址"), this);
    formLayout->addWidget(m_rememberServerCheckBox);
    
    // 登录和注册按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_loginButton = new QPushButton(tr("登录"), this);
    m_loginButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                 "QPushButton:hover { background-color: #2980b9; }");
    connect(m_loginButton, &QPushButton::clicked, this, &FfmpegClientWidget::onLoginClicked);
    
    m_registerButton = new QPushButton(tr("注册"), this);
    m_registerButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                    "QPushButton:hover { background-color: #229954; }");
    connect(m_registerButton, &QPushButton::clicked, this, &FfmpegClientWidget::onShowRegisterPage);
    
    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_registerButton);
    formLayout->addLayout(buttonLayout);
    
    loginLayout->addWidget(loginGroup);
    
    // 状态标签
    m_loginStatusLabel = new QLabel(tr("准备登录"), this);
    m_loginStatusLabel->setAlignment(Qt::AlignCenter);
    m_loginStatusLabel->setStyleSheet("color: #7f8c8d;");
    loginLayout->addWidget(m_loginStatusLabel);
    
    loginLayout->addStretch();
    
    // 支持键盘快捷登录
    connect(m_usernameEdit, &QLineEdit::returnPressed, m_passwordEdit, QOverload<>::of(&QLineEdit::setFocus));
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &FfmpegClientWidget::onLoginClicked);
    
    m_stackedLayout->addWidget(m_loginWidget);
}

void FfmpegClientWidget::initializeRegisterPage()
{
    m_registerWidget = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(m_registerWidget);
    
    // 标题
    QLabel *titleLabel = new QLabel(tr("FFmpeg Media Server - 注册"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50; margin: 20px;");
    registerLayout->addWidget(titleLabel);
    
    // 注册表单
    QGroupBox *registerGroup = new QGroupBox(tr("创建账户"), this);
    QVBoxLayout *formLayout = new QVBoxLayout(registerGroup);
    
    // 服务器地址
    formLayout->addWidget(new QLabel(tr("服务器地址:")));
    m_regServerUrlEdit = new QLineEdit("http://localhost:8080", this);
    m_regServerUrlEdit->setPlaceholderText(tr("请输入服务器地址，例如: http://localhost:8080"));
    formLayout->addWidget(m_regServerUrlEdit);
    
    // 用户名
    formLayout->addWidget(new QLabel(tr("用户名:")));
    m_regUsernameEdit = new QLineEdit(this);
    m_regUsernameEdit->setPlaceholderText(tr("请输入用户名，至少3个字符"));
    formLayout->addWidget(m_regUsernameEdit);
    
    // 邮箱
    formLayout->addWidget(new QLabel(tr("邮箱:")));
    m_regEmailEdit = new QLineEdit(this);
    m_regEmailEdit->setPlaceholderText(tr("请输入邮箱地址"));
    m_regEmailEdit->setStyleSheet("QLineEdit { font-family: monospace; }");
    formLayout->addWidget(m_regEmailEdit);
    
    // 密码
    formLayout->addWidget(new QLabel(tr("密码:")));
    m_regPasswordEdit = new QLineEdit(this);
    m_regPasswordEdit->setEchoMode(QLineEdit::Password);
    m_regPasswordEdit->setPlaceholderText(tr("请输入密码，至少6个字符"));
    formLayout->addWidget(m_regPasswordEdit);
    
    // 确认密码
    formLayout->addWidget(new QLabel(tr("确认密码:")));
    m_regConfirmPasswordEdit = new QLineEdit(this);
    m_regConfirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_regConfirmPasswordEdit->setPlaceholderText(tr("再次输入密码"));
    formLayout->addWidget(m_regConfirmPasswordEdit);
    
    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_submitRegisterButton = new QPushButton(tr("注册"), this);
    m_submitRegisterButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                         "QPushButton:hover { background-color: #229954; }");
    connect(m_submitRegisterButton, &QPushButton::clicked, this, &FfmpegClientWidget::onRegisterClicked);
    
    m_backToLoginButton = new QPushButton(tr("返回登录"), this);
    m_backToLoginButton->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; font-weight: bold; padding: 10px; border-radius: 5px; }"
                                     "QPushButton:hover { background-color: #7f8c8d; }");
    connect(m_backToLoginButton, &QPushButton::clicked, this, &FfmpegClientWidget::onShowLoginPage);
    
    buttonLayout->addWidget(m_submitRegisterButton);
    buttonLayout->addWidget(m_backToLoginButton);
    formLayout->addLayout(buttonLayout);
    
    registerLayout->addWidget(registerGroup);
    
    // 状态标签
    m_registerStatusLabel = new QLabel(tr("准备注册"), this);
    m_registerStatusLabel->setAlignment(Qt::AlignCenter);
    m_registerStatusLabel->setStyleSheet("color: #7f8c8d;");
    registerLayout->addWidget(m_registerStatusLabel);
    
    registerLayout->addStretch();
    
    // 支持键盘快捷注册
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
    
    // ��ǩҳ
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
    
    // 命令基本信息
    QGroupBox *basicGroup = new QGroupBox(tr("基本信息"), this);
    QGridLayout *basicLayout = new QGridLayout(basicGroup);
    
    basicLayout->addWidget(new QLabel(tr("命令类型:")), 0, 0);
    m_commandTypeCombo = new QComboBox(this);
    m_commandTypeCombo->addItems({tr("视频转换"), tr("音频转换"), tr("视频压缩"), tr("视频合并"), tr("截图"), tr("自定义")});
    basicLayout->addWidget(m_commandTypeCombo, 0, 1);
    
    basicLayout->addWidget(new QLabel(tr("命令名称:")), 1, 0);
    m_commandNameEdit = new QLineEdit(this);
    m_commandNameEdit->setPlaceholderText(tr("请输入命令名称"));
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
    
    // 命令参数
    QGroupBox *paramGroup = new QGroupBox(tr("命令参数"), this);
    QVBoxLayout *paramLayout = new QVBoxLayout(paramGroup);
    
    m_commandLineEdit = new QTextEdit(this);
    m_commandLineEdit->setMaximumHeight(100);
    m_commandLineEdit->setPlaceholderText(tr("请输入FFmpeg命令行参数"));
    paramLayout->addWidget(m_commandLineEdit);
    
    m_parametersEdit = new QLineEdit(this);
    m_parametersEdit->setPlaceholderText(tr("附加参数 (JSON格式)"));
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
    
    // 命令类型变化事件
    connect(m_commandTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index < m_commandPresets.size()) {
            const auto &preset = m_commandPresets[index];
            m_commandNameEdit->setText(preset.name);
            m_commandLineEdit->setText(preset.commandLine);
        }
    });
    
    // 初始化时禁用执行按钮，直到登录成功
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
        showSuccessMessage(tr("文件列表已刷新"));
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
    
    m_downloadButton = new QPushButton(tr("下载所选文件"), this);
    connect(m_downloadButton, &QPushButton::clicked, this, &FfmpegClientWidget::onDownloadFileClicked);
    listLayout->addWidget(m_downloadButton);
    
    layout->addWidget(listGroup);
    
    // 初始化时禁用文件操作控件
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
    
    m_clearHistoryButton = new QPushButton(tr("清除历史"), this);
    connect(m_clearHistoryButton, &QPushButton::clicked, this, [this]() {
        if (QMessageBox::question(this, tr("确认"), tr("确认要清除所有历史记录吗？")) == QMessageBox::Yes) {
            // TODO: 实现历史记录清除
            showSuccessMessage(tr("历史记录已清除"));
        }
    });
    toolLayout->addWidget(m_clearHistoryButton);
    
    toolLayout->addStretch();
    layout->addLayout(toolLayout);
    
    // 历史记录表格
    m_historyTable = new QTableWidget(this);
    m_historyTable->setColumnCount(8);
    QStringList headers = {tr("ID"), tr("命令名称"), tr("输入文件"), tr("输出文件"), tr("状态"), tr("结果"), tr("执行时间"), tr("错误信息")};
    m_historyTable->setHorizontalHeaderLabels(headers);
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(m_historyTable);
    
    // 初始化时禁用历史记录按钮
    m_refreshHistoryButton->setEnabled(false);
}

void FfmpegClientWidget::setupCommandPresets()
{
    m_commandPresets = {
        {tr("视频转换"), "-i {input} -c:v libx264 -c:a aac {output}", tr("将视频转换为H.264+AAC格式")},
        {tr("音频转换"), "-i {input} -vn -c:a mp3 {output}", tr("提取并转换为MP3音频")},
        {tr("视频压缩"), "-i {input} -vf scale=1280:-1 -crf 23 {output}", tr("压缩视频到720p")},
        {tr("视频合并"), "-f concat -safe 0 -i filelist.txt -c copy {output}", tr("合并多个视频文件")},
        {tr("截图"), "-i {input} -ss 00:00:01 -vframes 1 {output}", tr("从视频中提取一帧")}
    };
}

void FfmpegClientWidget::showLoginPage()
{
    m_stackedLayout->setCurrentWidget(m_loginWidget);
    setLoginEnabled(true);
}

void FfmpegClientWidget::showRegisterPage()
{
    m_stackedLayout->setCurrentWidget(m_registerWidget);
    setRegisterEnabled(true);
}

void FfmpegClientWidget::showMainPage()
{
    m_stackedLayout->setCurrentWidget(m_mainWidget);
    setLoginEnabled(false);
    setRegisterEnabled(false);
    setCommandEnabled(true);
    
    // ������ʷ��¼
    onCommandHistoryClicked();
}

void FfmpegClientWidget::setLoginEnabled(bool enabled)
{
    if (m_loginButton) m_loginButton->setEnabled(enabled);
    if (m_registerButton) m_registerButton->setEnabled(enabled);
    if (m_usernameEdit) m_usernameEdit->setEnabled(enabled);
    if (m_passwordEdit) m_passwordEdit->setEnabled(enabled);
    if (m_serverUrlEdit) m_serverUrlEdit->setEnabled(enabled);
}

void FfmpegClientWidget::setRegisterEnabled(bool enabled)
{
    if (m_submitRegisterButton) m_submitRegisterButton->setEnabled(enabled);
    if (m_backToLoginButton) m_backToLoginButton->setEnabled(enabled);
    if (m_regUsernameEdit) m_regUsernameEdit->setEnabled(enabled);
    if (m_regEmailEdit) m_regEmailEdit->setEnabled(enabled);
    if (m_regPasswordEdit) m_regPasswordEdit->setEnabled(enabled);
    if (m_regConfirmPasswordEdit) m_regConfirmPasswordEdit->setEnabled(enabled);
    if (m_regServerUrlEdit) m_regServerUrlEdit->setEnabled(enabled);
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

void FfmpegClientWidget::onShowRegisterPage() 
 { 
     // 预填服务器地址 
     m_regServerUrlEdit->setText(m_serverUrlEdit->text().trimmed()); 
     
     // 清空注册表单 
     m_regUsernameEdit->clear(); 
     m_regEmailEdit->clear(); 
     m_regPasswordEdit->clear(); 
     m_regConfirmPasswordEdit->clear(); 
     
     m_registerStatusLabel->setText(tr("准备注册")); 
     
     // 显示注册页面 
     m_stackedLayout->setCurrentWidget(m_registerWidget); 
 } 
 
 void FfmpegClientWidget::onShowLoginPage() 
 { 
     // 预填服务器地址 
     m_serverUrlEdit->setText(m_regServerUrlEdit->text().trimmed()); 
     
     m_loginStatusLabel->setText(tr("准备登录")); 
     
     // 显示登录页面 
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
        showErrorMessage(tr("请输入服务器地址"));
        return;
    }
    
    if (username.isEmpty()) {
        showErrorMessage(tr("请输入用户名"));
        return;
    }
    
    if (username.length() < 3) {
        showErrorMessage(tr("用户名至少需要3个字符"));
        return;
    }
    
    if (email.isEmpty()) {
        showErrorMessage(tr("请输入邮箱地址"));
        return;
    }
    
    // 简单的邮箱格式验证
    if (!email.contains("@") || !email.contains(".")) {
        showErrorMessage(tr("请输入有效的邮箱地址"));
        return;
    }
    
    if (password.isEmpty()) {
        showErrorMessage(tr("请输入密码"));
        return;
    }
    
    if (password.length() < 6) {
        showErrorMessage(tr("密码至少需要6个字符"));
        return;
    }
    
    if (password != confirmPassword) {
        showErrorMessage(tr("两次输入的密码不一致"));
        return;
    }
    
    setRegisterEnabled(false);
    m_registerStatusLabel->setText(tr("正在注册..."));
    
    sendRegisterRequest(username, password, email);
}

void FfmpegClientWidget::onRegisterReply() 
 { 
     if (!m_currentReply) { 
         qDebug() << "[FfmpegClientWidget] 注册响应为空"; 
         setRegisterEnabled(true); 
         return; 
     } 
     
     qDebug() << "[FfmpegClientWidget] 处理注册响应"; 
     logNetworkResponse(m_currentReply); 
     
     QByteArray responseData = m_currentReply->readAll(); 
     int statusCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); 
     int errorCode = m_currentReply->error(); 
     QString errorString = m_currentReply->errorString(); 
     
     qDebug() << "[FfmpegClientWidget] 注册响应数据大小:" << responseData.size() << "字节"; 
     qDebug() << "[FfmpegClientWidget] HTTP状态码:" << statusCode; 
     qDebug() << "[FfmpegClientWidget] 错误代码:" << errorCode; 
     qDebug() << "[FfmpegClientWidget] 响应内容:" << responseData.left(500); 
     
     m_currentReply->deleteLater(); 
     m_currentReply = nullptr; 
     
     QJsonParseError parseError; 
     QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError); 
     
     if (parseError.error != QJsonParseError::NoError) { 
         qDebug() << "[FfmpegClientWidget] JSON解析错误:" << parseError.errorString(); 
         showErrorMessage(tr("服务器响应格式错误")); 
         setRegisterEnabled(true); 
         return; 
     } 
     
     QJsonObject responseObj = doc.object(); 
     qDebug() << "[FfmpegClientWidget] 注册响应对象:" << responseObj;
    
    if (statusCode >= 200 && statusCode < 300) {
        if (responseObj.contains("success") && responseObj["success"].toBool()) {
            qDebug() << "[FfmpegClientWidget] 注册成功";
            showSuccessMessage(tr("注册成功，请登录"));
            
            // 预填登录信息
            m_serverUrlEdit->setText(m_regServerUrlEdit->text().trimmed());
            m_usernameEdit->setText(m_regUsernameEdit->text().trimmed());
            m_passwordEdit->clear();
            qDebug() << "[FfmpegClientWidget] 已预填登录信息";
            
            // 切换到登录页面
            onShowLoginPage();
        } else {
            QString errorMsg = tr("注册失败");
            if (responseObj.contains("message")) {
                errorMsg = responseObj["message"].toString();
            } else if (responseObj.contains("error")) {
                errorMsg = responseObj["error"].toString();
            }
            qDebug() << "[FfmpegClientWidget] 注册失败:" << errorMsg;
            showErrorMessage(errorMsg);
        }
    } else {
        QString errorMsg = tr("注册失败");
        if (responseObj.contains("message")) {
            errorMsg = responseObj["message"].toString();
        } else if (responseObj.contains("error")) {
            errorMsg = responseObj["error"].toString();
        } else {
            errorMsg = tr("HTTP错误: %1").arg(statusCode);
        }
        qDebug() << "[FfmpegClientWidget] 注册HTTP错误:" << errorMsg;
        showErrorMessage(errorMsg);
    }
    
    setRegisterEnabled(true);
    qDebug() << "[FfmpegClientWidget] 注册响应处理完成";
}

void FfmpegClientWidget::sendLoginRequest(const QString &username, const QString &password)
{
    qDebug() << "[FfmpegClientWidget] 开始登录请求";
    qDebug() << "[FfmpegClientWidget] 服务器URL:" << m_serverUrl;
    qDebug() << "[FfmpegClientWidget] 用户名:" << username;
    qDebug() << "[FfmpegClientWidget] 密码长度:" << password.length();
    
    QJsonObject requestObj;
    requestObj["username"] = username;
    requestObj["password"] = password;
    
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson();
    
    // 修复：确保URL格式正确
    QString serverUrl = m_serverUrl.trimmed();
    if (!serverUrl.startsWith("http://") && !serverUrl.startsWith("https://")) {
        serverUrl = "http://" + serverUrl;
        qDebug() << "[FfmpegClientWidget] 自动添加 http:// 前缀:" << serverUrl;
    }

    // 确保URL末尾没有斜杠，然后添加API路径
    if (serverUrl.endsWith("/")) {
        serverUrl.chop(1);
    }

    QUrl url(serverUrl + "/api/auth/login");
    qDebug() << "[FfmpegClientWidget] 构建的完整URL:" << url.toString();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    configureNetworkRequest(request);
    
    logNetworkRequest(request, requestData);
    
    if (m_currentReply) {
        qDebug() << "[FfmpegClientWidget] 取消之前的网络请求";
        m_currentReply->deleteLater();
    }
    
    m_currentReply = m_networkManager->post(request, requestData);
    connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onLoginReply);
    qDebug() << "[FfmpegClientWidget] 登录请求已发送";
}

void FfmpegClientWidget::onLoginReply()
{
    if (!m_currentReply) {
        qDebug() << "[FfmpegClientWidget] 登录响应为空";
        return;
    }
    
    qDebug() << "[FfmpegClientWidget] 处理登录响应";
    logNetworkResponse(m_currentReply);
    
    m_loginButton->setEnabled(true);
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        qDebug() << "[FfmpegClientWidget] 登录响应数据大小:" << responseData.size() << "字节";
        qDebug() << "[FfmpegClientWidget] 响应内容:" << responseData.left(500);
        
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "[FfmpegClientWidget] JSON解析错误:" << parseError.errorString();
            m_loginStatusLabel->setText(tr("响应格式错误"));
            showErrorMessage(tr("服务器响应格式错误"));
        } else {
            QJsonObject responseObj = doc.object();
            qDebug() << "[FfmpegClientWidget] 登录响应对象:" << responseObj;
            
            if (responseObj["success"].toBool()) {
                // 修复：从data字段中获取token
                QJsonObject dataObj = responseObj["data"].toObject();
                m_authToken = dataObj["token"].toString();
                qDebug() << "[FfmpegClientWidget] 登录成功，Token长度:" << m_authToken.length();
                setLoginStatus(true, m_usernameEdit->text());
                
                // 保存服务器地址和用户名
                if (m_rememberServerCheckBox->isChecked()) {
                    QSettings settings;
                    settings.setValue("serverUrl", m_serverUrl);
                    settings.setValue("username", m_usernameEdit->text());
                    qDebug() << "[FfmpegClientWidget] 已保存登录信息";
                }
            } else {
                // 修复：从data字段中获取message（如果存在）
                QString errorMsg;
                if (responseObj.contains("data")) {
                    QJsonObject dataObj = responseObj["data"].toObject();
                    errorMsg = dataObj["message"].toString();
                }
                if (errorMsg.isEmpty()) {
                    errorMsg = responseObj["message"].toString();
                }
                qDebug() << "[FfmpegClientWidget] 登录失败:" << errorMsg;
                m_loginStatusLabel->setText(tr("登录失败: %1").arg(errorMsg));
                showErrorMessage(errorMsg);
            }
        }
    } else {
        QString errorString = m_currentReply->errorString();
        int errorCode = m_currentReply->error();
        int statusCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        qDebug() << "[FfmpegClientWidget] 登录网络错误:";
        qDebug() << "[FfmpegClientWidget] 错误代码:" << errorCode;
        qDebug() << "[FfmpegClientWidget] HTTP状态码:" << statusCode;
        qDebug() << "[FfmpegClientWidget] 错误描述:" << errorString;
        
        m_loginStatusLabel->setText(tr("网络错误: %1").arg(errorString));
        showErrorMessage(tr("网络错误: %1").arg(errorString));
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    qDebug() << "[FfmpegClientWidget] 登录响应处理完成";
}

void FfmpegClientWidget::onExecuteCommandClicked() 
 { 
     const QString commandName = m_commandNameEdit->text().trimmed(); 
     const QString commandLine = m_commandLineEdit->toPlainText().trimmed(); 
     const QString inputFile = m_inputFileEdit->text().trimmed(); 
     const QString outputFile = m_outputFileEdit->text().trimmed(); 
     const QString parameters = m_parametersEdit->text().trimmed(); 
     
     qDebug() << "[FfmpegClientWidget] 开始执行命令"; 
     qDebug() << "[FfmpegClientWidget] 命令名称:" << commandName; 
     qDebug() << "[FfmpegClientWidget] 命令行:" << commandLine; 
     qDebug() << "[FfmpegClientWidget] 输入文件:" << inputFile; 
     qDebug() << "[FfmpegClientWidget] 输出文件:" << outputFile; 
     qDebug() << "[FfmpegClientWidget] 参数:" << parameters;
    
    if (commandName.isEmpty()) {
        qDebug() << "[FfmpegClientWidget] 命令名称为空";
        showErrorMessage(tr("请输入命令名称"));
        return;
    }
    
    if (commandLine.isEmpty()) {
        qDebug() << "[FfmpegClientWidget] 命令行参数为空";
        showErrorMessage(tr("请输入命令行参数"));
        return;
    }
    
    m_executeButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 不确定进度
    m_statusLabel->setText(tr("正在执行命令..."));
    
    sendExecuteCommandRequest(commandName, commandLine);
}

void FfmpegClientWidget::sendExecuteCommandRequest(const QString &commandName, const QString &commandLine) 
 { 
     qDebug() << "[FfmpegClientWidget] 准备执行命令"; 
     
     QJsonObject requestObj; 
     requestObj["commandName"] = commandName; 
     requestObj["commandLine"] = commandLine; 
     requestObj["inputFile"] = m_inputFileEdit->text(); 
     requestObj["outputFile"] = m_outputFileEdit->text(); 
     requestObj["parameters"] = m_parametersEdit->text(); 
     
     QJsonDocument doc(requestObj); 
     QByteArray requestData = doc.toJson(); 
     
     QUrl url(m_serverUrl + "/api/commands"); 
     QNetworkRequest request(url); 
     request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"); 
     request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8()); 
     
     qDebug() << "[FfmpegClientWidget] Token长度:" << m_authToken.length(); 
     logNetworkRequest(request, requestData); 
     
     if (m_currentReply) { 
         qDebug() << "[FfmpegClientWidget] 取消之前的网络请求"; 
         m_currentReply->deleteLater(); 
     } 
     
     m_currentReply = m_networkManager->post(request, requestData); 
     connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onCommandReply); 
     qDebug() << "[FfmpegClientWidget] 命令执行请求已发送"; 
 }

void FfmpegClientWidget::onCommandReply()
{
    if (!m_currentReply) {
        qDebug() << "[FfmpegClientWidget] 命令响应为空";
        return;
    }
    
    qDebug() << "[FfmpegClientWidget] 处理命令执行响应";
    logNetworkResponse(m_currentReply);
    
    m_executeButton->setEnabled(true);
    m_progressBar->setVisible(false);
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        qDebug() << "[FfmpegClientWidget] 命令响应数据大小:" << responseData.size() << "字节";
        qDebug() << "[FfmpegClientWidget] 响应内容:" << responseData.left(500);
        
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "[FfmpegClientWidget] JSON解析错误:" << parseError.errorString();
            m_statusLabel->setText(tr("响应格式错误"));
            showErrorMessage(tr("服务器响应格式错误"));
        } else {
            QJsonObject responseObj = doc.object();
            qDebug() << "[FfmpegClientWidget] 命令响应对象:" << responseObj;
            
            if (responseObj["success"].toBool()) {
                qDebug() << "[FfmpegClientWidget] 命令执行成功";
                m_statusLabel->setText(tr("命令执行成功"));
                showSuccessMessage(tr("命令已成功提交执行"));
                
                // 刷新历史记录
                onCommandHistoryClicked();
            } else {
                QString errorMsg = responseObj["message"].toString();
                qDebug() << "[FfmpegClientWidget] 命令执行失败:" << errorMsg;
                m_statusLabel->setText(tr("命令执行失败: %1").arg(errorMsg));
                showErrorMessage(errorMsg);
            }
        }
    } else {
        QString errorString = m_currentReply->errorString();
        int errorCode = m_currentReply->error();
        int statusCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        qDebug() << "[FfmpegClientWidget] 命令执行网络错误:";
        qDebug() << "[FfmpegClientWidget] 错误代码:" << errorCode;
        qDebug() << "[FfmpegClientWidget] HTTP状态码:" << statusCode;
        qDebug() << "[FfmpegClientWidget] 错误描述:" << errorString;
        
        m_statusLabel->setText(tr("网络错误: %1").arg(errorString));
        showErrorMessage(tr("网络错误: %1").arg(errorString));
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    qDebug() << "[FfmpegClientWidget] 命令响应处理完成";
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
    qDebug() << "[FfmpegClientWidget] 加载历史记录，Token:" << m_authToken; // 调试输出Token
    
    QUrl url(m_serverUrl + "/api/commands/user");
    QNetworkRequest request(url);
    
    // 关键修改：确保Token格式正确，即Bearer + 空格 + Token值
    QString authHeader = QString("Bearer %1").arg(m_authToken);
    request.setRawHeader("Authorization", authHeader.toUtf8());
    
    // 可选：设置超时和Content-Type为正确的标准值
    //request.setTransferTimeout(5000);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    // 打印请求头以便调试：
    qDebug() << "[FfmpegClientWidget] Authorization头:" << request.rawHeader("Authorization");
    
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
    
    m_currentReply = m_networkManager->get(request);
    connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onLoadCommandsReply);
}

void FfmpegClientWidget::onLoadCommandsReply()
{
    if (!m_currentReply) {
        qDebug() << "[FfmpegClientWidget] 加载历史响应为空";
        return;
    }
    
    qDebug() << "[FfmpegClientWidget] 处理加载历史响应";
    logNetworkResponse(m_currentReply);
    
    if (m_currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = m_currentReply->readAll();
        qDebug() << "[FfmpegClientWidget] 历史记录响应数据大小:" << responseData.size() << "字节";
        
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "[FfmpegClientWidget] JSON解析错误:" << parseError.errorString();
            showErrorMessage(tr("服务器响应格式错误"));
        } else {
            QJsonObject responseObj = doc.object();
            qDebug() << "[FfmpegClientWidget] 加载历史记录响应:" << responseObj;
            
            if (responseObj["success"].toBool()) {
                QJsonArray commands = responseObj["commands"].toArray();
                qDebug() << "[FfmpegClientWidget] 获取到" << commands.size() << "条历史记录";
                updateCommandHistory(commands);
            } else {
                QString errorMsg = responseObj["message"].toString();
                qDebug() << "[FfmpegClientWidget] 获取历史记录失败:" << errorMsg;
                showErrorMessage(errorMsg);
            }
        }
    } else {
        QString errorString = m_currentReply->errorString();
        int errorCode = m_currentReply->error();
        int statusCode = m_currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        qDebug() << "[FfmpegClientWidget] 获取历史记录网络错误:";
        qDebug() << "[FfmpegClientWidget] 错误代码:" << errorCode;
        qDebug() << "[FfmpegClientWidget] HTTP状态码:" << statusCode;
        qDebug() << "[FfmpegClientWidget] 错误描述:" << errorString;
        
        showErrorMessage(tr("获取历史记录失败: %1").arg(errorString));
    }
    
    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    qDebug() << "[FfmpegClientWidget] 加载历史响应处理完成";
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

void FfmpegClientWidget::sendRegisterRequest(const QString &username, const QString &password, const QString &email)
{
    m_serverUrl = m_regServerUrlEdit->text().trimmed();
    
    qDebug() << "[FfmpegClientWidget] 开始注册请求";
    qDebug() << "[FfmpegClientWidget] 服务器URL:" << m_serverUrl;
    qDebug() << "[FfmpegClientWidget] 用户名:" << username;
    qDebug() << "[FfmpegClientWidget] 邮箱:" << email;
    qDebug() << "[FfmpegClientWidget] 密码长度:" << password.length();
    
    QJsonObject requestObj;
    requestObj["username"] = username;
    requestObj["password"] = password;
    requestObj["email"] = email;
    
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson();
    
    QUrl url(m_serverUrl + "/api/auth/register");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    configureNetworkRequest(request);
    
    logNetworkRequest(request, requestData);
    
    if (m_currentReply) {
        qDebug() << "[FfmpegClientWidget] 取消之前的网络请求"; 
         m_currentReply->deleteLater(); 
     } 
     
     m_currentReply = m_networkManager->post(request, requestData); 
     connect(m_currentReply, &QNetworkReply::finished, this, &FfmpegClientWidget::onRegisterReply); 
     qDebug() << "[FfmpegClientWidget] 注册请求已发送"; 
 } 
 
 void FfmpegClientWidget::sendValidateTokenRequest()
{
    qDebug() << "[FfmpegClientWidget] 发送Token验证请求";
    
    // 修复：检查m_authToken是否为空
    if (m_authToken.isEmpty()) {
        qDebug() << "[FfmpegClientWidget] Token为空，不发送验证请求";
        return;
    }
    
    QUrl url(m_serverUrl + "/api/auth/validate");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    
    logNetworkRequest(request);
    
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &FfmpegClientWidget::onValidateTokenReply);
    qDebug() << "[FfmpegClientWidget] Token验证请求已发送";
}

void FfmpegClientWidget::onValidateTokenReply() 
 { 
     QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender()); 
     if (!reply) { 
         qDebug() << "[FfmpegClientWidget] Token验证响应为空"; 
         return; 
     } 
     
     qDebug() << "[FfmpegClientWidget] 处理Token验证响应"; 
     logNetworkResponse(reply); 
     
     if (reply->error() != QNetworkReply::NoError) { 
         int errorCode = reply->error(); 
         int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); 
         
         qDebug() << "[FfmpegClientWidget] Token验证失败:"; 
         qDebug() << "[FfmpegClientWidget]  错误代码:" << errorCode; 
         qDebug() << "[FfmpegClientWidget]  HTTP状态码:" << statusCode; 
         qDebug() << "[FfmpegClientWidget]  错误描述:" << reply->errorString(); 
         
         // 验证失败时，可能需要重新登录 
         if (reply->error() == QNetworkReply::AuthenticationRequiredError) { 
             qDebug() << "[FfmpegClientWidget] Token已过期，需要重新登录"; 
             logout(); 
         } 
     } else { 
         qDebug() << "[FfmpegClientWidget] Token验证成功"; 
         QByteArray responseData = reply->readAll(); 
         if (!responseData.isEmpty()) { 
             qDebug() << "[FfmpegClientWidget] 验证响应:" << responseData.left(200); 
         } 
     } 
     
     reply->deleteLater(); 
     qDebug() << "[FfmpegClientWidget] Token验证响应处理完成"; 
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

void FfmpegClientWidget::configureNetworkRequest(QNetworkRequest &request)
{
    // 配置SSL设置，用于测试环境
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);
}

void FfmpegClientWidget::configureNetworkDebugging()
{
    qDebug() << "[FfmpegClientWidget] 配置网络调试";
    
    // 捕获SSL错误
    connect(m_networkManager, &QNetworkAccessManager::sslErrors, 
            [](QNetworkReply *reply, const QList<QSslError> &errors) {
                qDebug() << "[Network] SSL错误:";
                for (const QSslError &error : errors) {
                    qDebug() << "[Network]  -" << error.errorString();
                }
            });
    
    // 检查代理设置
    QNetworkProxy proxy = QNetworkProxy::applicationProxy();
    if (proxy.type() != QNetworkProxy::NoProxy) {
        qDebug() << "[Network] 使用代理:" << proxy.hostName() << ":" << proxy.port();
    } else {
        qDebug() << "[Network] 未使用代理";
    }
    
    qDebug() << "[FfmpegClientWidget] 网络调试配置完成";
}

void FfmpegClientWidget::logNetworkRequest(const QNetworkRequest &request, const QByteArray &data)
{
    qDebug() << "[Network] =================== 网络请求 ===================";
    qDebug() << "[Network] URL:" << request.url().toString();
    qDebug() << "[Network] 方法:" << request.attribute(QNetworkRequest::CustomVerbAttribute).toString();
    
    // 打印请求头
    for (const auto &header : request.rawHeaderList()) {
        qDebug() << "[Network] Header:" << header << "=" << request.rawHeader(header);
    }
    
    // 打印请求体（限制长度）
    if (!data.isEmpty()) {
        QString dataStr = QString::fromUtf8(data.left(1000));
        if (data.size() > 1000) {
            dataStr += "...(truncated)";
        }
        qDebug() << "[Network] Body:" << dataStr;
        qDebug() << "[Network] Body size:" << data.size() << "bytes";
    }
    
    qDebug() << "[Network] ===============================================";
}

void FfmpegClientWidget::logNetworkResponse(QNetworkReply *reply)
{
    if (!reply) {
        qDebug() << "[Network] 响应为空";
        return;
    }
    
    qDebug() << "[Network] =================== 网络响应 ===================";
    qDebug() << "[Network] URL:" << reply->url().toString();
    qDebug() << "[Network] HTTP状态码:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "[Network] 错误代码:" << reply->error();
    qDebug() << "[Network] 错误描述:" << reply->errorString();
    
    // 打印响应头
    for (const auto &header : reply->rawHeaderList()) {
        qDebug() << "[Network] Response Header:" << header << "=" << reply->rawHeader(header);
    }
    
    qDebug() << "[Network] ===============================================";
}

void FfmpegClientWidget::loadCommandPresets()
{
    // 从文件加载命令预设
}
