#ifndef IPCMGRBASE_H
#define IPCMGRBASE_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QString>
#include <QByteArray>
#include <QThread>
#include <QWidget>

// 进程角色枚举（服务端=主EXE，客户端=功能EXE）
enum class IPCRole {
    Server,  // 服务端（监听连接、启动子进程）
    Client   // 客户端（主动连接服务端）
};

// 进程通信基类（可继承）
class IPCMgrBase : public QObject
{
    Q_OBJECT
public:

    // ========== 工具函数（子类可调用） ==========
    /**
     * @brief 激活当前窗口（跨平台）
     */
    void activateWindow();

    /**
     * @brief 构造函数
     * @param role 进程角色（Server/Client）
     * @param pipeName 通信管道名称（全局唯一，区分不同应用）
     * @param parent 父对象
     */
    explicit IPCMgrBase(IPCRole role, const QString& pipeName, QObject *parent = nullptr);
    virtual ~IPCMgrBase();

    // ========== 通用接口（子类直接调用） ==========
    /**
     * @brief 启动子进程（仅Server角色可用）
     * @param exePath 子EXE路径
     * @param hideCurrentWindow 启动后是否隐藏当前窗口
     * @return 是否启动成功
     */
    bool startChildProcess(const QString& exePath, bool hideCurrentWindow = true);

    /**
     * @brief 发送消息（Server/Client均可用）
     * @param msg 要发送的消息
     * @return 是否发送成功
     */
    bool sendMessage(const QString& msg);

    /**
     * @brief 连接服务端（仅Client角色可用）
     * @return 是否连接成功
     */
    bool connectToServer();

    /**
     * @brief 设置当前窗口（用于激活/显示）
     * @param window 窗口指针
     */
    void setCurrentWindow(QWidget* window) { m_currentWindow = window; }

    void onSocketError(QAbstractSocket::SocketError error);
signals:
    void messageReceived(const QString& msg);
    void childProcessExited(int exitCode);
    void connectSuccess();
    void connectFailed();

protected slots:
    // ========== 钩子函数（子类重写实现业务逻辑） ==========
    /**
     * @brief 接收消息回调（子类重写解析消息）
     * @param msg 收到的消息
     */
    virtual void onMessageReceived(const QString& msg) { emit messageReceived(msg); }

    /**
     * @brief 子进程退出回调（仅Server角色，子类重写处理）
     * @param exitCode 退出码
     */
    virtual void onChildProcessExited(int exitCode) { emit childProcessExited(exitCode); }

    /**
     * @brief 连接成功回调（子类重写）
     */
    virtual void onConnectSuccess() { emit connectSuccess(); }

    /**
     * @brief 连接失败回调（子类重写）
     */
    virtual void onConnectFailed() { emit connectFailed(); }

protected:


private slots:
    // ========== 内部槽函数（基类实现通用逻辑） ==========
    // Server：新客户端连接
    void onNewClientConnected();
    // 接收消息（Server/Client）
    void onReadyRead();
    // 连接断开

    void onDisconnected();
    // Server：子进程退出
    void onChildProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    IPCRole m_role;                  // 进程角色
    QString m_pipeName;              // 通信管道名称（全局唯一）
    QLocalServer* m_localServer;     // Server：本地通信服务器
    QLocalSocket* m_localSocket;     // Server/Client：通信套接字
    QProcess* m_childProcess;        // Server：子进程对象
    QWidget* m_currentWindow = nullptr; // 当前窗口（用于激活/显示）
};

#endif // IPCMGRBASE_H
