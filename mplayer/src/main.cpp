#include <QApplication>
#include "mplayer.h"
#include "myipcmgr.h"
#include <QDebug>

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
    
    // 连接到主进程
    QObject::connect(&ipcClient, &MyIPCMgr::messageReceived, [&](const QString& msg){
        if (msg.startsWith("play_video:")) {
            QString path = msg.mid(11);
            mp.playVideo(path);
        }
    });

    if (ipcClient.connectToServer()) {
        qDebug() << "MPlayer connected to server successfully";
    } else {
        qDebug() << "MPlayer failed to connect to server, maybe started independently";
    }

    mp.show();

    return app.exec();
}

