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
    , m_localServer(nullptr)
    , m_localSocket(nullptr)
{

    if (m_role == IPCRole::Server) {

        m_localServer = new QLocalServer(this);

        if (QLocalServer::removeServer(m_pipeName)) {
            qDebug() << "Removed existing pipe:" << m_pipeName;
        }
        // ��������
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


//Socket�������ۺ���

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

    // ���ݹܵ����Ƹ��ӽ���,���ӽ���֪�������ĸ��ܵ�

    QStringList args;
    args << m_pipeName;

    if (hideCurrentWindow && m_currentWindow) {
        m_currentWindow->hide();
    }

    m_childProcess->start(exePath, args);

    // �ȴ�����

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
    // ������Ϣ���ӻ��з���Ϊ�ָ���������ճ����
    QByteArray data = msg.toUtf8() + "\n";
    qint64 written = socket->write(data);
    socket->flush(); // ��������
    return written == data.size();
}

// Client�����ӷ����
bool IPCMgrBase::connectToServer()
{
    if (m_role != IPCRole::Client) {
        qDebug() << "Only Client can connect to server";
        return false;
    }
    m_localSocket->connectToServer(m_pipeName);
    // �ȴ����ӣ���ʱ3�룩
    return m_localSocket->waitForConnected(3000);
}

// ���ǰ���ڣ���ƽ̨��
void IPCMgrBase::activateWindow()
{
    if (!m_currentWindow) return;
    m_currentWindow->show();
    m_currentWindow->raise();       // ����������
    m_currentWindow->activateWindow(); // �����
}

// ========== �ڲ��ۺ���ʵ�� ==========
// Server���¿ͻ�������
void IPCMgrBase::onNewClientConnected()
{
    // ������һ���ͻ�������
    if (m_localSocket) {
        m_localSocket->disconnectFromServer();
        m_localSocket->deleteLater();
    }
    m_localSocket = m_localServer->nextPendingConnection();
    // �޸�5��readyRead/disconnected�źŸ��þɰ�connect�﷨
    connect(m_localSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_localSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    qDebug() << "New client connected";
}

// ������Ϣ��Server/Clientͨ�ã�
void IPCMgrBase::onReadyRead()
{
    QLocalSocket* socket = (m_role == IPCRole::Server) ? m_localSocket : m_localSocket;
    if (!socket) return;
    // ��ȡ�������ݣ������з��ָ
    QByteArray data = socket->readAll();
    QStringList msgs = QString::fromUtf8(data).split("\n", QString::SkipEmptyParts);
    for (const QString& msg : msgs) {
        // ����������д�Ļص�����
        onMessageReceived(msg);
    }
}

// ���ӶϿ�
void IPCMgrBase::onDisconnected()
{
    qDebug() << "Socket disconnected";
    if (m_role == IPCRole::Server) {
        // Server���ͻ��˶Ͽ��󼤻���������
        activateWindow();
    }
}

// Server���ӽ����˳�
void IPCMgrBase::onChildProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Child process exited, code:" << exitCode;
    // ��������ص�
    onChildProcessExited(exitCode);
    // ������������
    activateWindow();
}
