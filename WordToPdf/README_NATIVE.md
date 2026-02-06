# WordToPdf Native 项目

## 概述

WordToPdf Native 项目提供了用于 Word 和 PDF 格式转换的原生 C++ 接口。该项目包含两个主要组件：

1. **WordToPdf.py** - Python 实现的转换工具
2. **WordToPdfNative.dll** - C++ 实现的 Native DLL，提供 C 接口

## 项目结构

```
WordToPdf/
├── WordToPdf.py                    # Python 转换工具
├── WordToPdf.pyproj               # Python 项目文件
├── WordToPdfNative.vcxproj        # C++ 项目文件
├── WordToPdfNative.h              # 公共头文件
├── WordToPdfNative.cpp            # 主要实现文件
├── converter_wrapper.h            # C 包装器头文件
├── converter_wrapper.c            # C 包装器实现
├── WordToPdfNativeTest.cpp        # 使用示例
└── README_NATIVE.md               # 本文件
```

## 编译要求

### C++ 项目
- Visual Studio 2017 或更高版本
- Windows SDK
- C++17 支持

### Python 项目
- Python 3.7+
- 依赖库：`docx2pdf`, `pdf2docx`, `python-docx`, `reportlab`

## 使用方法

### 1. 编译 C++ DLL

在 Visual Studio 中打开 `MultiMediaTool.sln`，选择 `WordToPdfNative` 项目，然后编译。

### 2. 使用 C++ 接口

```cpp
#include <windows.h>
#include <iostream>
#include "WordToPdfNative.h"  // 包含头文件

int main() {
    // 直接调用函数（静态链接）
    if (ConvertWordToPdf("input.docx", "output.pdf") == 0) {
        std::cout << "转换成功" << std::endl;
    }
    
    return 0;
}
```

### 3. 动态加载使用

```cpp
#include <windows.h>
#include <iostream>

// 定义函数指针
typedef int (*ConvertWordToPdfFunc)(const char*, const char*);

int main() {
    HMODULE hDll = LoadLibrary(L"WordToPdfNative.dll");
    ConvertWordToPdfFunc ConvertWordToPdf = 
        (ConvertWordToPdfFunc)GetProcAddress(hDll, "ConvertWordToPdf");
    
    if (ConvertWordToPdf("input.docx", "output.pdf") == 0) {
        std::cout << "转换成功" << std::endl;
    }
    
    FreeLibrary(hDll);
    return 0;
}
```

## API 参考

### ConvertWordToPdf
```c
int ConvertWordToPdf(const char* input_path, const char* output_path);
```
将 Word 文档 (.docx) 转换为 PDF。

**参数：**
- `input_path`: 输入的 Word 文档路径
- `output_path`: 输出的 PDF 文件路径

**返回值：** 0 表示成功，非 0 表示失败

### ConvertPdfToWord
```c
int ConvertPdfToWord(const char* input_path, const char* output_path);
```
将 PDF 文档转换为 Word 文档 (.docx)。

**参数：**
- `input_path`: 输入的 PDF 文件路径
- `output_path`: 输出的 Word 文档路径

**返回值：** 0 表示成功，非 0 表示失败

### IsWordInstalled
```c
int IsWordInstalled();
```
检查系统是否安装了 Microsoft Word。

**返回值：** 1 表示已安装，0 表示未安装

### WordToPdf_GetLastError
```c
const char* WordToPdf_GetLastError();
```
获取最后一次操作的错误信息。

**返回值：** 错误信息字符串，如果没有错误则返回空字符串

### ClearLastError
```c
void ClearLastError();
```
清除错误信息。

## 注意事项

1. **Word 转 PDF** 需要系统安装 Microsoft Word
2. **PDF 转 Word** 使用 pdf2docx 库，支持大多数 PDF 格式
3. 如果 Word 未安装，会使用备用方案（仅支持纯文本转换）
4. 建议以管理员权限运行以避免权限问题

## 依赖项

### 运行时依赖
- Microsoft Word（用于 DOCX 转 PDF）
- Python 运行时（如果使用 Python 后端）

### 编译时依赖
- Windows SDK
- Visual C++ 运行时库

## 故障排除

### 常见问题

1. **"无法加载 WordToPdfNative.dll"**
   - 检查 DLL 文件是否存在
   - 确保 DLL 在正确的路径或系统 PATH 中

2. **"Word 转 PDF 失败"**
   - 确保 Microsoft Word 已正确安装
   - 尝试以管理员权限运行
   - 检查 Word 进程是否被占用

3. **"PDF 转 Word 失败"**
   - 检查 PDF 文件是否受密码保护
   - 确保输入的 PDF 文件格式正确

## 许可证

本项目遵循主项目的许可证条款。