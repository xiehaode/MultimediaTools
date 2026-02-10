#pragma once
#include <string>
#include <windows.h>  // 核心头文件，包含宽/多字节转换API
#include <cstdio>     // 用于日志打印（可根据你的log替换）

// 日志宏（可替换为你项目中的logi/loge）
#define LOG_INFO(fmt, ...)  printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)

std::string utf8_to_gbk(const std::string& utf8_str);


/**
 * @brief GBK 转 UTF-8（Windows 平台）
 * @param gbk_str 输入：GBK 编码的 std::string（必须是合法GBK字节序列，Windows中文系统）
 * @return 输出：UTF-8 编码的 std::string；转换失败返回空字符串
 */
std::string gbk_to_utf8(const std::string& gbk_str);
