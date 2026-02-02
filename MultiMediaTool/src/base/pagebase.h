#ifndef PAGEBASE_H
#define PAGEBASE_H
#include <QWidget>
#include "src/base/timedcmdexecutor.h"
#include "src/base/ipcmgrbase.h"

class pageBase : public QWidget
{
    Q_OBJECT
public:
    pageBase(QWidget *parent = nullptr);
    virtual ~pageBase();

    // IPC 接口封装
    bool sendMessage(const QString& msg);
    bool startChildProcess(const QString& exePath, bool hideCurrentWindow = true);
    void setCurrentWindow(QWidget* window);

    // Command 接口封装
    QVariantMap executeCmdSyncWithTime(const QString &cmd, int timeout = -1);
    void executeCmdAsyncWithTime(const QString &cmd);

protected slots:
    // 内部槽函数，用于转发信号到虚拟函数
    virtual void onMessageReceived(const QString& msg);
    virtual void onChildProcessExited(int exitCode);
    virtual void onConnectSuccess();
    virtual void onConnectFailed();

protected:
    TimedCmdExecutor* m_cmdExecutor;
    IPCMgrBase* m_ipcMgr;
};

#endif // PAGEBASE_H
