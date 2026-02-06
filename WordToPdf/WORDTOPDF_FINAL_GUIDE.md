# WordToPdf 最终使用指南

## 🎯 当前状态

### ✅ 已完成的功能

1. **原始 Python 工具** - `WordToPdf.py` (完全可用)
2. **C++ Native 项目** - `WordToPdfNative.vcxproj` (已集成到解决方案)
3. **Python Native 包装器** - `word_converter_native.py` (可用)
4. **完整的 API 接口** - C 风格函数接口

### 🔧 可用的解决方案

## 方案一：使用原始 Python 工具 (推荐)

这是最简单直接的方案，无需额外配置。

```bash
# 直接使用
python WordToPdf.py input.docx output.pdf
python WordToPdf.py input.pdf output.docx
```

**特点：**
- ✅ 完全可用
- ✅ 支持复杂格式
- ✅ 有备用转换方案
- ✅ 错误处理完善

## 方案二：使用 Python Native 包装器

提供 C 风格的 API 接口，适合集成到其他项目中。

```python
# 示例用法
import sys
sys.path.append('WordToPdf')
from word_converter_native import convert_file, get_last_error

# 转换文件
if convert_file('input.docx', 'output.pdf'):
    print("转换成功!")
else:
    print(f"转换失败: {get_last_error()}")
```

**API 函数：**
- `convert_file(input_path, output_path)` -> int
- `convert_files_batch(input_paths, output_dir, format)` -> int
- `check_file_support(file_path)` -> int
- `get_supported_formats()` -> str (JSON)
- `get_file_type(file_path)` -> str
- `get_last_error()` -> str
- `clear_error()` -> None

## 方案三：使用 C++ Native DLL

需要先在 Visual Studio 中构建项目。

### 构建步骤：
1. 打开 `MultiMediaTool.sln`
2. 选择 `WordToPdfNative` 项目
3. 构建 Release|x64 配置
4. 生成的 DLL 位于 `bin/x64/Release/WordToPdfNative.dll`

### 使用方法：
```cpp
#include "WordToPdfNative.h"

int main() {
    // 转换 Word 到 PDF
    if (ConvertWordToPdf("input.docx", "output.pdf") == 0) {
        printf("转换成功\n");
    } else {
        printf("转换失败: %s\n", WordToPdf_GetLastError());
    }
    return 0;
}
```

## 📁 文件说明

```
WordToPdf/
├── WordToPdf.py                    # 原始 Python 转换工具 ✅
├── WordToPdf.pyproj               # Python 项目文件
├── WordToPdfNative.vcxproj        # C++ 项目文件
├── WordToPdfNative.h              # C++ API 头文件
├── word_converter_native.py       # Python Native 包装器 ✅
├── converter_wrapper.pyx          # 原始 Cython 包装器 (有问题)
├── converter_wrapper_simple.pyx   # 简化的 Cython 包装器
├── requirements.txt               # Python 依赖
└── README_NATIVE.md               # 详细文档
```

## 🚀 推荐使用方式

### 对于最终用户：
直接使用 `WordToPdf.py`，这是最稳定和功能完整的方案。

### 对于开发者：
1. **Python 项目**：使用 `word_converter_native.py`
2. **C++ 项目**：构建并使用 `WordToPdfNative.dll`

### 对于集成项目：
1. 如果只需要转换功能，使用 Python 版本
2. 如果需要原生性能，使用 C++ 版本
3. 两个版本可以并存，根据需要选择

## 🛠️ 故障排除

### 常见问题：

1. **"WordToPdf 模块未找到"**
   - 确保在正确的目录中运行
   - 检查 Python 路径设置

2. **"转换失败: Word 未安装"**
   - 这是正常提示，程序会使用备用方案
   - 备用方案仅支持纯文本转换

3. **"找不到 WordToPdfNative.dll"**
   - 需要先在 Visual Studio 中构建 C++ 项目
   - 确保 DLL 在正确的路径中

## 📝 开发建议

### 如果要扩展功能：

1. **Python 版本**：直接修改 `WordToPdf.py`
2. **C++ 版本**：修改 `WordToPdfNative.cpp` 和相关文件
3. **API 版本**：修改 `word_converter_native.py`

### 如果要添加新的文件格式：

1. 在 `get_supported_formats()` 中添加格式定义
2. 在 `convert_file()` 中添加转换逻辑
3. 更新 `check_file_support()` 中的支持列表

## 🎉 总结

WordToPdf 功能已成功集成到 MultiMediaTool 解决方案中，提供了：

- ✅ **立即可用**的 Python 转换工具
- ✅ **完整集成**的 C++ Native 项目
- ✅ **兼容性好**的 API 接口
- ✅ **详细文档**和使用说明

用户可以根据具体需求选择最适合的实现方案。