#ifndef MPLAYERMANAGER_H
#define MPLAYERMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QProcess>
#include "src/base/cmdexecutor.h"

// MPlayer进程管理器
// 负责启动mplayer进程、传递参数、监控状态
class MPlayerManager : public QObject
{
    Q_OBJECT

public:
    explicit MPlayerManager(QObject *parent = nullptr);
    ~MPlayerManager();

    // ========== 进程管理 ==========
    // 启动mplayer进程
    // 参数：filePath - 视频文件路径；args - 额外参数列表
    // 返回值：true表示启动成功，false表示失败
    bool startMPlayer(const QString &filePath, const QStringList &args = QStringList());
    
    // 停止mplayer进程
    void stopMPlayer();
    
    // 检查mplayer是否正在运行
    bool isRunning() const;
    
    // ========== 进程通信 ==========
    // 向mplayer发送命令
    // 参数：command - 要发送的命令
    void sendCommand(const QString &command);
    
    // ========== 状态查询 ==========
    // 获取当前播放状态
    enum PlaybackState {
        Idle,       // 空闲
        Loading,    // 加载中
        Playing,    // 播放中
        Paused,     // 暂停
        Stopped,    // 停止
        Error       // 错误
    };
    PlaybackState getState() const;
    
    // 获取当前文件路径
    QString getCurrentFile() const;
    
    // ========== 静态方法 ==========
    // 获取mplayer可执行文件路径
    static QString getMPlayerPath();
    
    // 检查mplayer是否可用
    static bool isMPlayerAvailable();

signals:
    // 进程状态信号
    void mPlayerStarted(const QString &filePath);
    void mPlayerStopped();
    void mPlayerError(const QString &errorMessage);
    
    // 播放状态信号
    void playbackStateChanged(PlaybackState state);
    void positionChanged(qint64 position, qint64 duration);
    void mediaFinished();
    
    // 输出信号
    void outputReceived(const QString &output);
    void errorReceived(const QString &error);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString &output);
    void onProcessError();
    void checkProcessStatus();

private:
    void parseOutput(const QString &output);
    void updateState(PlaybackState newState);
    
    CmdExecutor *m_executor;
    QProcess *m_process;
    QTimer *m_statusTimer;
    
    PlaybackState m_currentState;
    QString m_currentFile;
    qint64 m_currentPosition;
    qint64 m_duration;
    
    static const int STATUS_CHECK_INTERVAL = 1000; // 1秒检查一次状态
};

#endif // MPLAYERMANAGER_H