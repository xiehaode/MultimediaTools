#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

namespace Ui {
class login;
}

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();

    // 获取用户信息
    QString getUsername() const;
    QString getToken() const;
    QString getRole() const;
    
    // 检查是否登录成功
    bool isLoggedIn() const;

signals:
    void loginSuccess(const QString& username, const QString& token, const QString& role);
    void loginFailed(const QString& error);
    void logoutRequested();

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void on_showPassword_toggled(bool checked);
    void on_rememberMe_toggled(bool checked);
    
    // 网络响应处理
    void handleLoginResponse(QNetworkReply* reply);
    void handleRegisterResponse(QNetworkReply* reply);
    
    // 定时器事件
    void checkServerConnection();
    void onConnectionTimeout();

private:
    Ui::login *ui;
    
    // 网络管理
    QNetworkAccessManager* networkManager;
    QTimer* connectionTimer;
    
    // 配置
    QString serverUrl;
    int connectionTimeout;
    
    // 登录状态
    QString currentUser;
    QString currentToken;
    QString currentRole;
    bool loginSuccessful;
    
    // 方法
    void initUI();
    void initNetwork();
    void loadSettings();
    void saveSettings();
    void validateInputs();
    void attemptLogin();
    void attemptRegistration();
    
    // 网络请求
    void sendLoginRequest(const QString& username, const QString& password);
    void sendRegisterRequest(const QString& username, const QString& email, const QString& password);
    
    // 工具方法
    QString hashPassword(const QString& password, const QString& salt = "");
    QString generateSalt();
    bool validateEmail(const QString& email);
    bool validatePassword(const QString& password);
    void showMessage(const QString& message, bool isError = false);
    void clearForm();
    void setLoadingState(bool loading);
    
    // 错误处理
    void handleNetworkError(QNetworkReply::NetworkError error);
    void parseErrorResponse(const QByteArray& data);
};

#endif // LOGIN_H