#include "IPCMgrBase.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QDebug>
#include <QAbstractSocket>

IPCMgrBase::IPCMgrBase(IPCRole role, const QString& pipeName, QObject *parent)
    : QObject(parent)
    , m_role(role)
    , m_pipeName(pipeName)
    , m_childProcess(new QProcess(this))
    , m_localServer(nullptr)  // 修复：显式初始化Server指针
    , m_localSocket(nullptr)  // 修复：显式初始化Socket指针
{

    if (m_role == IPCRole::Server) {

        m_localServer = new QLocalServer(this);

        if (QLocalServer::removeServer(m_pipeName)) {
            qDebug() << "Removed existing pipe:" << m_pipeName;
        }
        // 监听连接
        if (!m_localServer->listen(m_pipeName)) {
            qDebug() << "Server listen failed:" << m_localServer->errorString();
        } else {
            connect(m_localServer, SIGNAL(newConnection()), this, SLOT(onNewClientConnected()));
        }

        connect(m_childProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
                this, SLOT(onChildProcessFinished(int, QProcess::ExitStatus)));
    } else {

        m_localSocket = new QLocalSocket(this);

        connect(m_localSocket, SIGNAL(connected()), this, SLOT(onConnectSuccess()));
        connect(m_localSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    }
}


void IPCMgrBase::onSocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "Client connect failed:" << error;
    this->onConnectFailed();
}

IPCMgrBase::~IPCMgrBase()
{

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

    QStringList args;
    args << m_pipeName;

    if (hideCurrentWindow && m_currentWindow) {
        m_currentWindow->hide();
    }

    m_childProcess->start(exePath, args);
    if (!m_childProcess->waitForStarted(3000)) {
        qDebug() << "Start child process failed:" << m_childProcess->errorString();
        return false;
    }
    return true;
}

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
