// WordToPdfNativeTest.cpp : 示例文件，展示如何使用 WordToPdfNative.dll
#include <iostream>
#include <windows.h>

// 定义函数指针类型
typedef int (*ConvertWordToPdfFunc)(const char*, const char*);
typedef int (*ConvertPdfToWordFunc)(const char*, const char*);
typedef int (*IsWordInstalledFunc)();
typedef const char* (*WordToPdfGetLastErrorFunc)();
typedef void (*ClearLastErrorFunc)();

int main()
{
    // 加载 DLL
    HMODULE hDll = LoadLibrary(L"WordToPdfNative.dll");
    if (!hDll) {
        std::cerr << "无法加载 WordToPdfNative.dll" << std::endl;
        return 1;
    }

    // 获取函数指针
    ConvertWordToPdfFunc ConvertWordToPdf = (ConvertWordToPdfFunc)GetProcAddress(hDll, "ConvertWordToPdf");
    ConvertPdfToWordFunc ConvertPdfToWord = (ConvertPdfToWordFunc)GetProcAddress(hDll, "ConvertPdfToWord");
    IsWordInstalledFunc IsWordInstalled = (IsWordInstalledFunc)GetProcAddress(hDll, "IsWordInstalled");
    WordToPdfGetLastErrorFunc WordToPdfGetLastError = (WordToPdfGetLastErrorFunc)GetProcAddress(hDll, "WordToPdf_GetLastError");
    ClearLastErrorFunc ClearLastError = (ClearLastErrorFunc)GetProcAddress(hDll, "ClearLastError");

    if (!ConvertWordToPdf || !ConvertPdfToWord || !IsWordInstalled || !WordToPdfGetLastError || !ClearLastError) {
        std::cerr << "无法获取 DLL 函数地址" << std::endl;
        FreeLibrary(hDll);
        return 1;
    }

    // 检查 Word 是否安装
    if (IsWordInstalled()) {
        std::cout << "Microsoft Word 已安装" << std::endl;
    } else {
        std::cout << "Microsoft Word 未安装" << std::endl;
    }

    // 示例：Word 转 PDF
    std::cout << "尝试 Word 转 PDF..." << std::endl;
    int result = ConvertWordToPdf("test.docx", "output.pdf");
    if (result == 0) {
        std::cout << "Word 转 PDF 成功" << std::endl;
    } else {
        std::cerr << "Word 转 PDF 失败: " << WordToPdfGetLastError() << std::endl;
    }

    // 示例：PDF 转 Word
    std::cout << "尝试 PDF 转 Word..." << std::endl;
    result = ConvertPdfToWord("input.pdf", "output.docx");
    if (result == 0) {
        std::cout << "PDF 转 Word 成功" << std::endl;
    } else {
        std::cerr << "PDF 转 Word 失败: " << WordToPdfGetLastError() << std::endl;
    }

    // 清理
    FreeLibrary(hDll);
    return 0;
}