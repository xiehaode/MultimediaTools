#ifndef FFMPEGCLIENTWIDGET_H
#define FFMPEGCLIENTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QSplitter>
#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QTabWidget>
#include <memory>
#include <QStackedLayout>

class ApiManager;

class FfmpegClientWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FfmpegClientWidget(QWidget *parent = nullptr);
    ~FfmpegClientWidget();

    // 登录状态管理
    bool isLoggedIn() const;
    void setLoginStatus(bool logged, const QString &username = QString());
    void logout();
    
    // 服务器连接管理
    bool connectToServer(const QString &serverUrl, const QString &username, const QString &password);
    void disconnectFromServer();

private slots:
    // 登录相关
    void onLoginClicked();
    void onLogoutClicked();
    void onLoginReply();
    void onValidateTokenReply();
    
    // 注册相关
    void onRegisterClicked();
    void onShowRegisterPage();
    void onShowLoginPage();
    void onRegisterReply();
    
    // FFmpeg命令相关
    void onExecuteCommandClicked();
    void onCommandHistoryClicked();
    void onCommandReply();
    void onLoadCommandsClicked();
    void onLoadCommandsReply();
    
    // 文件操作相关
    void onUploadFileClicked();
    void onDownloadFileClicked();
    void onFileUploadReply();
    void onFileDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    
    // 界面更新
    void updateServerStatus();
    void onTimerTick();

private:
    // 初始化UI
    void initializeUI();
    void initializeLoginPage();
    void initializeRegisterPage();
    void initializeMainPage();
    void initializeCommandTab();
    void initializeFileTab();
    void initializeHistoryTab();
    
    // 网络请求
    void sendLoginRequest(const QString &username, const QString &password);
    void sendValidateTokenRequest();
    void sendRegisterRequest(const QString &username, const QString &password, const QString &email);
    void sendExecuteCommandRequest(const QString &commandName, const QString &commandLine);
    void sendLoadCommandsRequest();
    void sendFileUploadRequest(const QString &filePath);
    
    // UI状态管理
    void showLoginPage();
    void showRegisterPage();
    void showMainPage();
    void setLoginEnabled(bool enabled);
    void setRegisterEnabled(bool enabled);
    void setCommandEnabled(bool enabled);
    void updateCommandHistory(const QJsonArray &commands);
    
    // 错误处理
    void handleError(const QString &error, QNetworkReply *reply = nullptr);
    void showErrorMessage(const QString &message);
    void showSuccessMessage(const QString &message);
    
    // 网络配置
    void configureNetworkRequest(QNetworkRequest &request);

private:
    // 登录状态
    bool m_isLoggedIn;
    QString m_username;
    QString m_serverUrl;
    QString m_authToken;
    
    // API管理器
    std::unique_ptr<ApiManager> m_apiManager;
    
    // 网络管理
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply;
    
    // 定时器
    QTimer *m_statusTimer;
    
    // 主界面布局
    QVBoxLayout *m_mainLayout;
    QStackedLayout *m_stackedLayout;
    
    // 登录页面控件
    QWidget *m_loginWidget;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_serverUrlEdit;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
    QLabel *m_loginStatusLabel;
    QCheckBox *m_rememberServerCheckBox;
    
    // 注册页面控件
    QWidget *m_registerWidget;
    QLineEdit *m_regUsernameEdit;
    QLineEdit *m_regPasswordEdit;
    QLineEdit *m_regConfirmPasswordEdit;
    QLineEdit *m_regEmailEdit;
    QLineEdit *m_regServerUrlEdit;
    QPushButton *m_submitRegisterButton;
    QPushButton *m_backToLoginButton;
    QLabel *m_registerStatusLabel;
    
    // 主页面控件
    QWidget *m_mainWidget;
    QTabWidget *m_tabWidget;
    
    // 命令执行标签页
    QWidget *m_commandTab;
    QComboBox *m_commandTypeCombo;
    QLineEdit *m_commandNameEdit;
    QLineEdit *m_inputFileEdit;
    QLineEdit *m_outputFileEdit;
    QTextEdit *m_commandLineEdit;
    QLineEdit *m_parametersEdit;
    QPushButton *m_executeButton;
    QPushButton *m_browseInputButton;
    QPushButton *m_browseOutputButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    // 文件操作标签页
    QWidget *m_fileTab;
    QLineEdit *m_uploadFileEdit;
    QPushButton *m_uploadButton;
    QPushButton *m_browseUploadButton;
    QTableWidget *m_fileTable;
    QPushButton *m_downloadButton;
    QPushButton *m_refreshFilesButton;
    
    // 历史记录标签页
    QWidget *m_historyTab;
    QTableWidget *m_historyTable;
    QPushButton *m_refreshHistoryButton;
    QPushButton *m_clearHistoryButton;
    
    // 常用FFmpeg命令预设
    struct CommandPreset {
        QString name;
        QString commandLine;
        QString description;
    };
    QList<CommandPreset> m_commandPresets;
    
    void loadCommandPresets();
    void setupCommandPresets();
};

#endif // FFMPEGCLIENTWIDGET_H
