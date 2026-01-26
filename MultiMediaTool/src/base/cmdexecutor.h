#ifndef CMDEXECUTOR_H
#define CMDEXECUTOR_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QByteArray>

// 命令行执行基类（简化版，核心能力封装）
class CmdExecutor : public QObject
{
    Q_OBJECT
public:
    explicit CmdExecutor(QObject *parent = nullptr) : QObject(parent) {
        // 初始化进程，绑定输出/错误捕获
        m_process.setProcessChannelMode(QProcess::MergedChannels); // 合并标准输出和错误输出
        connect(&m_process, &QProcess::readyReadStandardOutput, this, [this]() {
            m_output += m_process.readAllStandardOutput(); // 累计输出内容
        });
        connect(&m_process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
            m_exitCode = exitCode;
            m_exitStatus = exitStatus;
            emit cmdFinished(exitCode, exitStatus, m_output); // 触发执行完成信号
        });
    }

    // ========== 同步执行命令（阻塞式，适合简单命令） ==========
    // 返回值：命令执行结果（输出内容）
    // 参数：cmd - 要执行的命令行；timeout - 超时时间（毫秒，-1表示不限时）
    QString executeCmdSync(const QString &cmd, int timeout = -1) {
        m_output.clear(); // 清空上次输出
        m_exitCode = -1;
        m_exitStatus = QProcess::CrashExit;

        // 启动命令行（Windows用cmd /c，Linux/mac用sh -c）
#ifdef Q_OS_WIN
        m_process.start("cmd", {"/c", cmd});
#else
        m_process.start("sh", {"-c", cmd});
#endif

        // 等待执行完成（阻塞）
        m_process.waitForFinished(timeout);

        // 返回输出内容（转成UTF-8避免中文乱码）
        return QString::fromLocal8Bit(m_output);
    }

    // ========== 异步执行命令（非阻塞，适合耗时命令） ==========
    void executeCmdAsync(const QString &cmd) {
        m_output.clear();
        m_exitCode = -1;
        m_exitStatus = QProcess::CrashExit;

#ifdef Q_OS_WIN
        m_process.start("cmd", {"/c", cmd});
#else
        m_process.start("sh", {"-c", cmd});
#endif
        m_process.closeWriteChannel(); // 关闭写入通道，避免进程阻塞
    }

    // 获取命令执行退出码（0表示成功，非0为错误）
    int exitCode() const { return m_exitCode; }
    // 获取命令执行状态（正常退出/崩溃）
    QProcess::ExitStatus exitStatus() const { return m_exitStatus; }

signals:
    // 异步执行完成信号（exitCode=退出码，exitStatus=退出状态，output=输出内容）
    void cmdFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString &output);

private:
    QProcess m_process;       // 进程对象
    QByteArray m_output;      // 命令输出缓存
    int m_exitCode = -1;      // 退出码
    QProcess::ExitStatus m_exitStatus = QProcess::CrashExit; // 退出状态
};

#endif // CMDEXECUTOR_H
