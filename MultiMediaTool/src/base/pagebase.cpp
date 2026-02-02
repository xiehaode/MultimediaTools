#include "src/base/pagebase.h"
#include <QDebug>

pageBase::pageBase(QObject *parent)
    : QObject(parent)
{
    // 初始化组件
    m_cmdExecutor = new TimedCmdExecutor(this);
    m_ipcMgr = new IPCMgrBase(IPCRole::Server, "MyApp_Unique_Pipe", this);


    // 连接 IPC 信号到虚拟函数槽
    connect(m_ipcMgr, SIGNAL(messageReceived(QString)), this, SLOT(onMessageReceived(QString)));
    connect(m_ipcMgr, SIGNAL(childProcessExited(int)), this, SLOT(onChildProcessExited(int)));
    connect(m_ipcMgr, SIGNAL(connectSuccess()), this, SLOT(onConnectSuccess()));
    connect(m_ipcMgr, SIGNAL(connectFailed()), this, SLOT(onConnectFailed()));
}

pageBase::~pageBase()
{
}

bool pageBase::sendMessage(const QString &msg)
{
    return m_ipcMgr->sendMessage(msg);
}

bool pageBase::startChildProcess(const QString &exePath, bool hideCurrentWindow)
{
    return m_ipcMgr->startChildProcess(exePath, hideCurrentWindow);
}

void pageBase::setCurrentWindow(QWidget *window)
{
    m_ipcMgr->setCurrentWindow(window);
}

QVariantMap pageBase::executeCmdSyncWithTime(const QString &cmd, int timeout)
{
    return m_cmdExecutor->executeCmdSyncWithTime(cmd, timeout);
}

void pageBase::executeCmdAsyncWithTime(const QString &cmd)
{
    m_cmdExecutor->executeCmdAsyncWithTime(cmd);
}

void pageBase::onMessageReceived(const QString& msg)
{
    qDebug() << "pageBase received msg:" << msg;
    // 例如：处理子进程的退出通知
    if (msg == "FuncExe_Exit") {
        m_ipcMgr->activateWindow();
    }
}

void pageBase::onChildProcessExited(int exitCode)
{
    qDebug() << "子进程退出，码：" << exitCode;
}

void pageBase::onConnectSuccess()
{
}

void pageBase::onConnectFailed()
{
}
