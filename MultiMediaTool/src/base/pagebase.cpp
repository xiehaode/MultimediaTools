#include "pagebase.h"

pageBase::pageBase(QWidget *parent)
    : IPCMgrBase(IPCRole::Server, "MyApp_Unique_Pipe", parent) // 关键：初始化父类
{
    // 子类自身的初始化逻辑
    this->setCurrentWindow(parent); // 若parent是窗口，设置激活窗口
}

void pageBase::onMessageReceived(const QString& msg)
{
    qDebug() << "pageBase received msg:" << msg;
    // 例如：处理子进程的退出通知
    if (msg == "FuncExe_Exit") {
        this->activateWindow();
    }
}
void pageBase::onChildProcessExited(int exitCode)
{
    qDebug() << "子进程退出，码：" << exitCode;
}
