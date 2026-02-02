#ifndef MYIPCMGR_H
#define MYIPCMGR_H

#include "src/base/ipcmgrbase.h"
#include <QWidget>
#include <QApplication>
#include <QDebug>

// 自定义IPC子类，继承IPCMgrBase
class MyIPCMgr : public IPCMgrBase
{
    Q_OBJECT
public:
    // 构造函数：传递角色、管道名、父对象，可扩展自定义参数（如主窗口）
    explicit MyIPCMgr(IPCRole role, const QString& pipeName, QWidget* mainWindow = nullptr, QObject *parent = nullptr);

protected:
    // 重写基类虚钩子函数，实现业务逻辑（核心）
    // 1. 接收进程间消息的回调
    void onMessageReceived(const QString& msg) override;

    // 2. 子进程退出的回调（仅Server角色生效）
    void onChildProcessExited(int exitCode) override;

    // 3. 连接服务端成功的回调（仅Client角色生效）
    void onConnectSuccess() override;

    // 4. 连接服务端失败的回调（仅Client角色生效）
    void onConnectFailed() override;
};

#endif // MYIPCMGR_H

