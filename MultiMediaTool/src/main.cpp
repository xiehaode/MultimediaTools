#include "gui/application.h"
#include "src/utils/myipcmgr.h"
#include <QApplication>
#include <QTranslator>
#include <QCommandLineParser>
#include <QDebug>

#include <windows.h>
#include <dbghelp.h>
#include <iostream>
#include <shlobj.h> // 用于创建目录
#include <string>

/** *************************************************************/
// 如果不存在会递归创建
void CreateFullDirectory(std::wstring path) {
    size_t pos = 0;
    do {
        pos = path.find_first_of(L"\\/", pos + 1);
        CreateDirectory(path.substr(0, pos).c_str(), NULL);
    } while (pos != std::wstring::npos);
}

void CreateDumpFile(EXCEPTION_POINTERS* pException) {

    // 方案 A: 自动获取 %LOCALAPPDATA% 路径
    wchar_t path[MAX_PATH];
    ExpandEnvironmentStrings(L"%LOCALAPPDATA%\\TestCrash\\", path, MAX_PATH);

    // 方案 B: 指定路径
    // wcscpy_s(path, L"D:\\MyDebugDumps\\");

    // 1. 确保目标文件夹存在
    CreateFullDirectory(path);

    // 2. 构造带时间戳的文件名，避免覆盖
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t fileName[MAX_PATH];
    swprintf_s(fileName, L"%lsCrash_%04d%02d%02d_%02d%02d%02d.dmp",
        path, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    // 3. 创建文件
    HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
        dumpInfo.ThreadId = GetCurrentThreadId();
        dumpInfo.ExceptionPointers = pException;
        dumpInfo.ClientPointers = TRUE;

        BOOL bSuccess = MiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MiniDumpNormal,
            &dumpInfo,
            NULL,
            NULL
        );

        if (bSuccess) {
            std::wcout << L"Dump saved to: " << fileName << std::endl;
        }
        CloseHandle(hFile);
    }
}

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo) {
    CreateDumpFile(ExceptionInfo);
    return EXCEPTION_EXECUTE_HANDLER;
}





extern const QString IPC_PIPE_NAME = "MultiMediaTool-Unique-IPC-Pipe-2026";


int main(int argc, char *argv[])
{


    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

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


