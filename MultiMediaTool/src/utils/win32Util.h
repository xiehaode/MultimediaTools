#ifndef WIN32UTIL_H
#define WIN32UTIL_H

#include <windows.h>
#include <TlHelp32.h>
#include <QDebug>
#include <QString>
#include <Psapi.h>
#include <dbghelp.h>
#include <QDebug>
// 获取指定模块的基地址
uint64_t getModuleBaseAddress(const QString& moduleName) {
    // 获取当前进程句柄
    HANDLE hProcess = GetCurrentProcess();
    // 模块信息缓冲区
    HMODULE hModules[1024];
    DWORD cbNeeded;

    // 枚举当前进程加载的所有模块
    if (EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            char szModuleName[MAX_PATH] = {0};
            // 获取模块路径/名称
            GetModuleBaseNameA(hProcess, hModules[i], szModuleName, sizeof(szModuleName));
            // 匹配模块名（忽略大小写）
            if (QString(szModuleName).compare(moduleName, Qt::CaseInsensitive) == 0) {
                // 获取模块基地址
                MODULEINFO moduleInfo;
                GetModuleInformation(hProcess, hModules[i], &moduleInfo, sizeof(moduleInfo));
                qDebug()<<"ProcessModules information:"<<moduleInfo.lpBaseOfDll;
                return (uint64_t)moduleInfo.lpBaseOfDll;
            }
        }
    }
    return 0; // 未找到
}
#endif // WIN32UTIL_H
