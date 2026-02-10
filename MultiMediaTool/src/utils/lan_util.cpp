#include "lan_util.h"

std::string utf8_to_gbk(const std::string &utf8_str)
{
    if (utf8_str.empty()) return "";
    // 1. UTF-8 → 宽字符（移除MB_ERR_INVALID_CHARS，避免ASCII误判）
    int wchar_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), NULL, 0);
    if (wchar_len <= 0) return "";
    wchar_t* wchar_buf = new wchar_t[wchar_len + 1];
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), (int)utf8_str.size(), wchar_buf, wchar_len);
    wchar_buf[wchar_len] = L'\0';

    // 2. 宽字符 → GBK（移除WC_ERR_INVALID_CHARS，避免1004错误）
    int gbk_len = WideCharToMultiByte(CP_ACP, 0, wchar_buf, wchar_len, NULL, 0, NULL, NULL);
    if (gbk_len <= 0) { delete[] wchar_buf; return ""; }
    char* gbk_buf = new char[gbk_len + 1];
    WideCharToMultiByte(CP_ACP, 0, wchar_buf, wchar_len, gbk_buf, gbk_len, NULL, NULL);
    gbk_buf[gbk_len] = '\0';

    std::string gbk_str(gbk_buf);
    delete[] wchar_buf;
    delete[] gbk_buf;
    return gbk_str;
}

std::string gbk_to_utf8(const std::string &gbk_str)
{
    // 1. 空值校验
    if (gbk_str.empty())
    {
        LOG_ERROR("gbk_to_utf8: input string is empty");
        return "";
    }

    // 2. GBK → 宽字符（wchar_t，UTF-16 LE）
    // 先获取转换宽字符所需的缓冲区长度
    int wchar_len = MultiByteToWideChar(
        CP_ACP,                // 源编码：CP_ACP=GBK（Windows中文系统）
        MB_ERR_INVALID_CHARS,  // 遇到无效GBK字符则报错
        gbk_str.c_str(),       // 源GBK字符串
        (int)gbk_str.length(), // 源字符串长度（字节数，不含\0）
        NULL,
        0
    );
    if (wchar_len <= 0)
    {
        LOG_ERROR("gbk_to_utf8: MultiByteToWideChar failed, err=%d", GetLastError());
        return "";
    }

    // 分配宽字符缓冲区并执行转换
    wchar_t* wchar_buf = new wchar_t[wchar_len + 1];
    if (!wchar_buf)
    {
        LOG_ERROR("gbk_to_utf8: new wchar_t buffer failed");
        return "";
    }
    MultiByteToWideChar(
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        gbk_str.c_str(),
        (int)gbk_str.length(),
        wchar_buf,
        wchar_len
    );
    wchar_buf[wchar_len] = L'\0';

    // 3. 宽字符 → UTF-8（多字节）
    // 先获取转换UTF-8所需的缓冲区长度
    int utf8_len = WideCharToMultiByte(
        CP_UTF8,               // 目标编码：UTF-8
        WC_ERR_INVALID_CHARS,  // 遇到无效宽字符则报错
        wchar_buf,             // 源宽字符字符串
        wchar_len,             // 源宽字符长度（不含\0）
        NULL,
        0,
        NULL,
        NULL
    );
    if (utf8_len <= 0)
    {
        LOG_ERROR("gbk_to_utf8: WideCharToMultiByte failed, err=%d", GetLastError());
        delete[] wchar_buf;
        return "";
    }

    // 分配UTF-8缓冲区并执行转换
    char* utf8_buf = new char[utf8_len + 1];
    if (!utf8_buf)
    {
        LOG_ERROR("gbk_to_utf8: new char buffer failed");
        delete[] wchar_buf;
        return "";
    }
    WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        wchar_buf,
        wchar_len,
        utf8_buf,
        utf8_len,
        NULL,
        NULL
    );
    utf8_buf[utf8_len] = '\0';

    // 4. 封装结果并释放缓冲区
    std::string utf8_str(utf8_buf);
    delete[] wchar_buf;
    delete[] utf8_buf;

    return utf8_str;
}
