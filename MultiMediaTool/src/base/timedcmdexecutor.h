#ifndef TIMEDCMDEXECUTOR_H
#define TIMEDCMDEXECUTOR_H

#include "src/base/cmdexecutor.h"
#include <QElapsedTimer>
#include <QString>
#include <QVariantMap>

// 带执行时间记录的命令行执行子类
class TimedCmdExecutor : public CmdExecutor
{
    Q_OBJECT
public:
    explicit TimedCmdExecutor(QObject *parent = nullptr) : CmdExecutor(parent) {
        // 绑定基类的完成信号，用于异步执行的计时结束
        connect(this, &CmdExecutor::cmdFinished, this, [this](int exitCode, QProcess::ExitStatus exitStatus, const QString &output) {
            if (m_timer.isValid()) {
                m_lastExecuteTimeMs = m_timer.elapsed(); // 记录异步执行耗时（毫秒）
                m_timer.invalidate(); // 重置计时器
            }
            // 触发带计时的完成信号
            emit cmdFinishedWithTime(exitCode, exitStatus, output, m_lastExecuteTimeMs);
        });
    }

    // ========== 同步执行（带计时，返回结构化结果） ==========
    // 返回值：包含执行结果的QVariantMap，键说明：
    // - "output": 命令输出内容（QString）
    // - "exitCode": 退出码（int）
    // - "exitStatus": 退出状态（int，对应QProcess::ExitStatus）
    // - "elapsedMs": 执行耗时（毫秒，qint64）
    // - "success": 是否执行成功（bool，exitCode=0且exitStatus=NormalExit）
    QVariantMap executeCmdSyncWithTime(const QString &cmd, int timeout = -1) {
        QVariantMap result;
        QElapsedTimer timer;
        timer.start(); // 启动计时

        // 调用基类同步执行接口
        QString output = executeCmdSync(cmd, timeout);
        qint64 elapsedMs = timer.elapsed(); // 结束计时

        // 封装结构化结果
        result["output"] = output;
        result["exitCode"] = exitCode();
        result["exitStatus"] = static_cast<int>(exitStatus());
        result["elapsedMs"] = elapsedMs;
        result["success"] = (exitCode() == 0 && exitStatus() == QProcess::NormalExit);

        // 记录最后一次执行时间（供外部获取）
        m_lastExecuteTimeMs = elapsedMs;

        return result;
    }

    // ========== 异步执行（带计时，无返回值，通过信号返回结果） ==========
    void executeCmdAsyncWithTime(const QString &cmd) {
        m_timer.restart(); // 启动异步计时
        executeCmdAsync(cmd); // 调用基类异步执行接口
    }

    // 获取最后一次执行的耗时（毫秒）
    qint64 lastExecuteTimeMs() const { return m_lastExecuteTimeMs; }

signals:
    // 异步执行完成信号（带耗时）
    // 参数：exitCode-退出码，exitStatus-退出状态，output-输出内容，elapsedMs-耗时（毫秒）
    void cmdFinishedWithTime(int exitCode, QProcess::ExitStatus exitStatus, const QString &output, qint64 elapsedMs);

private:
    QElapsedTimer m_timer;       // 异步执行计时器
    qint64 m_lastExecuteTimeMs = 0; // 最后一次执行耗时（毫秒）
};

#endif // TIMEDCMDEXECUTOR_H
