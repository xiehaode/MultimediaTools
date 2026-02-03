#include "myipcmgr.h"

MyIPCMgr::MyIPCMgr(IPCRole role, const QString& pipeName, QWidget* mainWindow, QObject *parent)
    : IPCMgrBase(role, pipeName, parent)
{
    if (mainWindow) {
        setCurrentWindow(mainWindow);
    }
}

void MyIPCMgr::onMessageReceived(const QString& msg)
{
    // 业务解析：如根据消息指令执行窗口激活、功能调用等
    qDebug() << "收到跨进程消息：" << msg;
    if (msg == "activate_main_window") {
        activateWindow(); // 调用基类工具函数，激活主窗口
    } else if (msg == "exit_app") {
        qApp->exit(0); // 退出应用
    }
    // 也可转发基类的messageReceived信号，让其他模块处理
    emit messageReceived(msg);
}

// 2. 子进程退出的回调（仅Server角色生效）
void MyIPCMgr::onChildProcessExited(int exitCode)
{
    qDebug() <<"子进程退出 退出码为:"<< exitCode;
    // 业务处理：如重启子进程、更新状态等
    emit childProcessExited(exitCode);
}

// 3. 连接服务端成功的回调（仅Client角色生效）
void MyIPCMgr::onConnectSuccess()
{
    qDebug() << "连接主进程成功！";
    // 连接成功后可立即发送初始化消息
    sendMessage("client_init_ok");
    emit connectSuccess();
}

// 4. 连接服务端失败的回调（仅Client角色生效）
void MyIPCMgr::onConnectFailed()
{
    qDebug() << "连接主进程失败！";
    emit connectFailed();
}

