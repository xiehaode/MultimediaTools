#include "IPCMgrBase.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>
// 新增：包含QAbstractSocket头文件（解决SocketError枚举问题）
#include <QAbstractSocket>

IPCMgrBase::IPCMgrBase(IPCRole role, const QString& pipeName, QObject *parent)
    : QObject(parent)
    , m_role(role)
    , m_pipeName(pipeName)
    , m_childProcess(new QProcess(this))
    , m_localServer(nullptr)  // 修复：显式初始化Server指针
    , m_localSocket(nullptr)  // 修复：显式初始化Socket指针
{
    // 初始化通信组件
    if (m_role == IPCRole::Server) {
        // Server：创建本地服务器
        m_localServer = new QLocalServer(this);
        // 若管道已存在，先移除再监听
        if (QLocalServer::removeServer(m_pipeName)) {
            qDebug() << "Removed existing pipe:" << m_pipeName;
        }
        // 监听连接
        if (!m_localServer->listen(m_pipeName)) {
            qDebug() << "Server listen failed:" << m_localServer->errorString();
        } else {
            // 修复1：Qt 5.12兼容的connect语法（newConnection信号）
            connect(m_localServer, SIGNAL(newConnection()), this, SLOT(onNewClientConnected()));
        }
        // 修复2：QProcess::finished信号的兼容connect语法（解决重载匹配问题）
        connect(m_childProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(onChildProcessFinished(int, QProcess::ExitStatus)));
    } else {
        // Client：创建通信套接字
        m_localSocket = new QLocalSocket(this);
        // 修复3：connected信号的兼容语法
        connect(m_localSocket, SIGNAL(connected()), this, SLOT(onConnectSuccess()));
        // 修复4：error信号改用QAbstractSocket::SocketError，且用旧版语法
        connect(m_localSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    }
}

// 新增：Socket错误处理槽函数（替代原lambda表达式，兼容Qt 5.12）
void IPCMgrBase::onSocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "Client connect failed:" << error;
    this->onConnectFailed();
}

IPCMgrBase::~IPCMgrBase()
{
    // 清理资源
    if (m_role == IPCRole::Server && m_localServer && m_localServer->isListening()) {
        m_localServer->close();
    }
    if (m_localSocket && m_localSocket->state() == QLocalSocket::ConnectedState) {
        m_localSocket->disconnectFromServer();
    }
    if (m_childProcess && m_childProcess->state() == QProcess::Running) {
        m_childProcess->kill();
    }
}

// Server：启动子进程
bool IPCMgrBase::startChildProcess(const QString& exePath, bool hideCurrentWindow)
{
    if (m_role != IPCRole::Server) {
        qDebug() << "Only Server can start child process";
        return false;
    }
    if (m_childProcess->state() == QProcess::Running) {
        qDebug() << "Child process is already running";
        return false;
    }
    // 传递管道名称给子进程（让子进程知道连接哪个管道）
    QStringList args;
    args << m_pipeName;
    // 隐藏当前窗口（可选）
    if (hideCurrentWindow && m_currentWindow) {
        m_currentWindow->hide();
    }
    // 启动子进程
    m_childProcess->start(exePath, args);
    // 等待启动（超时3秒）
    if (!m_childProcess->waitForStarted(3000)) {
        qDebug() << "Start child process failed:" << m_childProcess->errorString();
        return false;
    }
    return true;
}

// 发送消息（Server/Client通用）
bool IPCMgrBase::sendMessage(const QString& msg)
{
    QLocalSocket* socket = (m_role == IPCRole::Server) ? m_localSocket : m_localSocket;
    if (!socket || socket->state() != QLocalSocket::ConnectedState) {
        qDebug() << "Socket not connected, send failed";
        return false;
    }
    // 发送消息（加换行符作为分隔符，避免粘包）
    QByteArray data = msg.toUtf8() + "\n";
    qint64 written = socket->write(data);
    socket->flush(); // 立即发送
    return written == data.size();
}

// Client：连接服务端
bool IPCMgrBase::connectToServer()
{
    if (m_role != IPCRole::Client) {
        qDebug() << "Only Client can connect to server";
        return false;
    }
    m_localSocket->connectToServer(m_pipeName);
    // 等待连接（超时3秒）
    return m_localSocket->waitForConnected(3000);
}

// 激活当前窗口（跨平台）
void IPCMgrBase::activateWindow()
{
    if (!m_currentWindow) return;
    m_currentWindow->show();
    m_currentWindow->raise();       // 提升到顶层
    m_currentWindow->activateWindow(); // 激活窗口
}

// ========== 内部槽函数实现 ==========
// Server：新客户端连接
void IPCMgrBase::onNewClientConnected()
{
    // 仅保留一个客户端连接
    if (m_localSocket) {
        m_localSocket->disconnectFromServer();
        m_localSocket->deleteLater();
    }
    m_localSocket = m_localServer->nextPendingConnection();
    // 修复5：readyRead/disconnected信号改用旧版connect语法
    connect(m_localSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_localSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    qDebug() << "New client connected";
}

// 接收消息（Server/Client通用）
void IPCMgrBase::onReadyRead()
{
    QLocalSocket* socket = (m_role == IPCRole::Server) ? m_localSocket : m_localSocket;
    if (!socket) return;
    // 读取所有数据（按换行符分割）
    QByteArray data = socket->readAll();
    QStringList msgs = QString::fromUtf8(data).split("\n", QString::SkipEmptyParts);
    for (const QString& msg : msgs) {
        // 调用子类重写的回调函数
        onMessageReceived(msg);
    }
}

// 连接断开
void IPCMgrBase::onDisconnected()
{
    qDebug() << "Socket disconnected";
    if (m_role == IPCRole::Server) {
        // Server：客户端断开后激活自身窗口
        activateWindow();
    }
}

// Server：子进程退出
void IPCMgrBase::onChildProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Child process exited, code:" << exitCode;
    // 调用子类回调
    onChildProcessExited(exitCode);
    // 激活自身窗口
    activateWindow();
}
