#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <memory>
#include "page/login.h"
#include "core/ApiManager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginSuccess(const QString& username, const QString& token, const QString& role);
    void onLoginFailed(const QString& error);
    void onLogoutRequested();
    void onNetworkError(const QString& error);
    void onServerStatusCheck();
    
    void showLoginPage();
    void showMainInterface();
    void onSettings();
    void onAbout();
    void onLogout();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupCentralWidget();
    void connectSignals();
    
    // UI组件
    QStackedWidget* stackedWidget;
    QWidget* mainWidget;
    
    // 页面
    login* loginPage;
    
    // 菜单和状态栏
    QLabel* statusLabel;
    QLabel* userLabel;
    QLabel* serverStatusLabel;
    QPushButton* logoutButton;
    QProgressBar* progressBar;
    
    // 定时器
    QTimer* statusTimer;
    
    // 状态
    bool isLoggedIn;
    QString currentUser;
    QString currentToken;
    QString currentUserRole;
};

#endif // MAINWINDOW_H