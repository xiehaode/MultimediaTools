// WordToPdfNative.cpp : 定义 DLL 应用程序的导出函数。
#include "pch.h"
#include "WordToPdfNative.h"
#include "converter_wrapper.h"

// 实现导出函数
WORDTOPDFNATIVE_API int ConvertWordToPdf(const char* input_path, const char* output_path)
{
    clear_error();
    return convert_file(input_path, output_path);
}

WORDTOPDFNATIVE_API int ConvertPdfToWord(const char* input_path, const char* output_path)
{
    clear_error();
    return convert_file(input_path, output_path);
}

WORDTOPDFNATIVE_API int IsWordInstalled()
{
    // 检查是否安装了 Microsoft Word
    clear_error();
    
    try {
        // 这里可以添加检查 Word 安装的逻辑
        // 暂时返回 1 表示支持
        return 1;
    }
    catch (...) {
        return 0;
    }
}

WORDTOPDFNATIVE_API const char* WordToPdf_GetLastError()
{
    return get_last_error();
}

WORDTOPDFNATIVE_API void ClearLastError()
{
    clear_error();
}