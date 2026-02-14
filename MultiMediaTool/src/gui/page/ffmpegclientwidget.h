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
    void onLoginReply();
    void onValidateTokenReply();
    
    // 注册相关
    void onRegisterClicked();
    void onShowRegisterPage();
    void onShowLoginPage();
    void onRegisterReply();

    // 登出相关
    void onLogoutClicked();
    
    // 界面更新
    void updateServerStatus();
    void onTimerTick();

private:
    // 初始化UI
    void initializeUI();
    void initializeLoginPage();
    void initializeRegisterPage();
    void initializeMainPage();
    
    // 网络请求
    void sendLoginRequest(const QString &username, const QString &password);
    void sendValidateTokenRequest();
    void sendRegisterRequest(const QString &serverUrl, const QString &username, const QString &email, const QString &password);
    
    // UI状态管理
    void showLoginPage();
    void showRegisterPage();
    void showMainPage();
    void setLoginEnabled(bool enabled);
    void setRegisterEnabled(bool enabled);
    
    // 错误处理
    void handleError(const QString &error, QNetworkReply *reply = nullptr);
    void showErrorMessage(const char *message);
    void showSuccessMessage(const char *message);
    
    // 网络配置
    void configureNetworkRequest(QNetworkRequest &request);
    void configureNetworkDebugging();
    void logNetworkRequest(const QNetworkRequest &request, const QByteArray &data = QByteArray());
    void logNetworkResponse(QNetworkReply *reply);

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
};

#endif // FFMPEGCLIENTWIDGET_H
