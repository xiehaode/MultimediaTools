#include "gui/application.h"
#include "src/utils/myipcmgr.h"
#include <QApplication>
#include <QTranslator>
#include <QCommandLineParser>
#include <QDebug>
#include <Windows.h>


extern const QString IPC_PIPE_NAME = "MultiMediaTool-Unique-IPC-Pipe-2026";


int main(int argc, char *argv[])
{
    //qt不能mat多线程否则会导致卡死
//    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
//    if (FAILED(hr)) {
//        qCritical() << "COM MTA模式初始化失败：" << hr;
//        return -1;
//    }
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    // ========== 命令行参数解析 ==========
    QCommandLineParser parser;
    QCommandLineOption clientOption("client", "Client Mode");
    parser.addOption(clientOption);
    parser.addPositionalArgument("pipeName", "Pipe Name", "[pipeName]");
    parser.process(a);



    // 检查是否已经有实例在运行（尝试作为Client连接）
    // 注意：如果是明确指定的子进程，或者手动启动的第二个实例，都先尝试连接
    MyIPCMgr ipcClient(IPCRole::Client, IPC_PIPE_NAME);
    if (ipcClient.connectToServer()) {
        qDebug() << "Connected to server, activating window and exiting";
        ipcClient.sendMessage("activate_main_window");
        return 0; // 退出当前实例
    }


    // 如果连接失败，说明当前没有运行中的主进程，则自身作为Server启动
    MainWindow w;
    // 与 basewindow 内部设置保持一致，避免覆盖掉最小化相关 flag
    w.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    w.resize(1000, 800);
    w.show();

    return a.exec();

    //CoUninitialize();
}


