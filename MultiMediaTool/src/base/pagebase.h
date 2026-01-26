#ifndef PAGEBASE_H
#define PAGEBASE_H
#include "src/base/timedcmdexecutor.h"
#include "src/base/ipcmgrbase.h"

class pageBase : public TimedCmdExecutor,IPCMgrBase
{
public:
    pageBase(QWidget *parent = nullptr);
    void onMessageReceived(const QString& msg) override;
    void onChildProcessExited(int exitCode) override;
};

#endif // PAGEBASE_H
