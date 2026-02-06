#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容

// Windows 头文件 - 使用条件编译避免在某些环境下找不到
#ifdef _WIN32
#include <windows.h>
#endif
