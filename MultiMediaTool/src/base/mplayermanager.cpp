#include "src/base/mplayermanager.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

MPlayerManager::MPlayerManager(QObject *parent)
    : QObject(parent)
    , m_executor(new CmdExecutor(this))
    , m_process(new QProcess(this))
    , m_statusTimer(new QTimer(this))
    , m_currentState(Idle)
    , m_currentPosition(0)
    , m_duration(0)
{
    // 设置进程
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    
    // 连接信号
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput());
        parseOutput(output);
        emit outputReceived(output);
    });
    
    connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
        QString error = QString::fromLocal8Bit(m_process->readAllStandardError());
        emit errorReceived(error);
    });
    
    connect(m_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &MPlayerManager::onProcessFinished);
    
    connect(m_process, &QProcess::errorOccurred, this, &MPlayerManager::onProcessError);
    
    // 状态检查定时器
    connect(m_statusTimer, &QTimer::timeout, this, &MPlayerManager::checkProcessStatus);
    m_statusTimer->setInterval(STATUS_CHECK_INTERVAL);
}

MPlayerManager::~MPlayerManager()
{
    stopMPlayer();
}

bool MPlayerManager::startMPlayer(const QString &filePath, const QStringList &args)
{
    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) {
        emit mPlayerError("Invalid file path: " + filePath);
        return false;
    }
    
    // 检查mplayer是否可用
    if (!isMPlayerAvailable()) {
        emit mPlayerError("MPlayer is not available. Please build mplayer first.");
        return false;
    }
    
    // 如果已经有进程在运行，先停止
    if (m_process->state() != QProcess::NotRunning) {
        stopMPlayer();
    }
    
    // 构建命令行参数
    QStringList mplayerArgs;
    
    // 基本参数
    mplayerArgs << "-quiet"           // 静默模式，减少输出
                << "-slave"           // 从模式，接受命令
                << "-idle"           // 空闲时不退出
                << "-really-quiet";    // 完全静默
    
    // 添加用户参数
    mplayerArgs.append(args);
    
    // 添加文件路径
    mplayerArgs << "\"" + filePath + "\"";
    
    // 启动进程
    QString mplayerPath = getMPlayerPath();
    qDebug() << "Starting MPlayer:" << mplayerPath << mplayerArgs;
    
    m_process->start(mplayerPath, mplayerArgs);
    
    if (!m_process->waitForStarted(5000)) {
        emit mPlayerError("Failed to start MPlayer: " + m_process->errorString());
        return false;
    }
    
    // 更新状态
    m_currentFile = filePath;
    updateState(Loading);
    
    // 开始状态检查
    m_statusTimer->start();
    
    emit mPlayerStarted(filePath);
    return true;
}

void MPlayerManager::stopMPlayer()
{
    if (m_process->state() != QProcess::NotRunning) {
        // 发送退出命令
        sendCommand("quit");
        
        // 等待进程退出
        if (!m_process->waitForFinished(3000)) {
            // 强制终止
            m_process->terminate();
            if (!m_process->waitForFinished(2000)) {
                m_process->kill();
                m_process->waitForFinished(1000);
            }
        }
    }
    
    m_statusTimer->stop();
    updateState(Stopped);
    m_currentFile.clear();
    m_currentPosition = 0;
    m_duration = 0;
}

bool MPlayerManager::isRunning() const
{
    return m_process->state() == QProcess::Running;
}

void MPlayerManager::sendCommand(const QString &command)
{
    if (isRunning()) {
        QByteArray cmd = (command + "\n").toLocal8Bit();
        m_process->write(cmd);
        qDebug() << "Sent command to MPlayer:" << command;
    }
}

MPlayerManager::PlaybackState MPlayerManager::getState() const
{
    return m_currentState;
}

QString MPlayerManager::getCurrentFile() const
{
    return m_currentFile;
}

QString MPlayerManager::getMPlayerPath()
{
    // 在开发环境中，mplayer应该位于相对于MultiMediaTool的目录
    QString basePath = QCoreApplication::applicationDirPath();
    QString mplayerPath;
    
#ifdef Q_OS_WIN
    mplayerPath = basePath + "/../../mplayer/debug/mplayer.exe";
    if (!QFileInfo::exists(mplayerPath)) {
        mplayerPath = basePath + "/../../mplayer/release/mplayer.exe";
    }
#else
    mplayerPath = basePath + "/../../mplayer/mplayer";
#endif
    
    return mplayerPath;
}

bool MPlayerManager::isMPlayerAvailable()
{
    return QFileInfo::exists(getMPlayerPath());
}

void MPlayerManager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString &output)
{
    Q_UNUSED(output)
    
    m_statusTimer->stop();
    
    qDebug() << "MPlayer process finished. Exit code:" << exitCode << "Status:" << exitStatus;
    
    if (exitCode != 0) {
        emit mPlayerError("MPlayer exited with error code: " + QString::number(exitCode));
    }
    
    updateState(Stopped);
    emit mPlayerStopped();
}

void MPlayerManager::onProcessError()
{
    QString errorMsg = "MPlayer process error: " + m_process->errorString();
    emit mPlayerError(errorMsg);
    updateState(Error);
}

void MPlayerManager::checkProcessStatus()
{
    if (!isRunning()) {
        m_statusTimer->stop();
        if (m_currentState != Stopped) {
            updateState(Stopped);
            emit mPlayerStopped();
        }
    }
}

void MPlayerManager::parseOutput(const QString &output)
{
    QStringList lines = output.split('\n');
    
    for (const QString &line : lines) {
        if (line.isEmpty()) continue;
        
        qDebug() << "MPlayer output:" << line;
        
        // 解析状态信息
        if (line.contains("ANS_LENGTH=")) {
            m_duration = line.split('=').last().toDouble() * 1000;
            emit positionChanged(m_currentPosition, m_duration);
        }
        else if (line.contains("ANS_TIME_POSITION=")) {
            m_currentPosition = line.split('=').last().toDouble() * 1000;
            emit positionChanged(m_currentPosition, m_duration);
        }
        else if (line.contains("Playing")) {
            updateState(Playing);
        }
        else if (line.contains("Paused")) {
            updateState(Paused);
        }
        else if (line.contains("ANS_pause=")) {
            bool isPaused = line.split('=').last() == "yes";
            updateState(isPaused ? Paused : Playing);
        }
        else if (line.contains("ANS_filename=")) {
            // 文件名信息
        }
        else if (line.contains("EOF code")) {
            // 播放结束
            updateState(Stopped);
            emit mediaFinished();
        }
        else if (line.contains("Error") || line.contains("Failed")) {
            emit mPlayerError(line);
            updateState(Error);
        }
    }
}

void MPlayerManager::updateState(PlaybackState newState)
{
    if (m_currentState != newState) {
        m_currentState = newState;
        emit playbackStateChanged(newState);
        qDebug() << "MPlayer state changed to:" << newState;
    }
}