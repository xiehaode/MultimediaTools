#include <QApplication>
#include "mplayer.h"
#include "myipcmgr.h"
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 获取管道名称（从命令行参数获取，基类startChildProcess会传过来）
    QString pipeName = "MultiMediaTool-Unique-IPC-Pipe-2026";
    if (argc > 1) {
        pipeName = argv[1];
    }
    
    mpalyer mp;
    
    // 创建Client角色的IPC实例
    MyIPCMgr ipcClient(IPCRole::Client, pipeName, &mp);
    
    // 标记是否已连接成功
    bool isConnected = false;
    
    // 连接到主进程
    QObject::connect(&ipcClient, &MyIPCMgr::messageReceived, [&](const QString& msg){
        qDebug() << "[MPlayer] 收到IPC消息:" << msg;
        if (msg.startsWith("play_video:")) {
            QString path = msg.mid(11);
            mp.playVideo(path);
        } else if (msg.startsWith("select_mode:")) {
            QString modeStr = msg.mid(12);
            qDebug() << "[MPlayer] 处理模式选择:" << modeStr;
            mp.selectModeFromString(modeStr);
        }
    });
    
    // 连接成功时的处理
    QObject::connect(&ipcClient, &MyIPCMgr::connectSuccess, [&]() {
        qDebug() << "[MPlayer] IPC连接成功";
        isConnected = true;
        // 连接成功后显示窗口
        mp.show();
    });
    
    // 连接失败时的处理
    QObject::connect(&ipcClient, &MyIPCMgr::connectFailed, [&]() {
        qDebug() << "[MPlayer] IPC连接失败，独立运行模式";
        // 连接失败也要显示窗口，但记录状态
        isConnected = false;
        mp.show();
    });

    // 尝试连接服务器
    qDebug() << "[MPlayer] 尝试连接到服务器，管道名:" << pipeName;
    if (ipcClient.connectToServer()) {
        qDebug() << "[MPlayer] 立即连接成功";
    } else {
        qDebug() << "[MPlayer] 连接超时，将独立运行";
        // 如果连接失败，也要显示窗口
        mp.show();
    }
    
    // 设置超时保护，5秒后强制显示窗口
    QTimer::singleShot(5000, [&]() {
        if (!mp.isVisible()) {
            qDebug() << "[MPlayer] 超时强制显示窗口";
            mp.show();
        }
    });

    return app.exec();
}

