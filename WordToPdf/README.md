# 通用文件格式转换器

支持 PDF、Word、HTML、PPT、图片(PNG/JPG/BMP)、CSV 之间的互转，并可打包为DLL供Qt使用。

## 功能特性

- 🔄 **多格式支持**: 支持6大类文件格式互转
- 📦 **打包为DLL**: 可编译为动态链接库供Qt/C++项目使用
- ⚡ **批量转换**: 支持批量文件转换
- 🛡️ **错误处理**: 完善的错误处理和日志记录
- 🔧 **备用方案**: 多种转换失败时的备用方案

## 支持的格式

### 输入格式
- **PDF**: `.pdf`
- **Word**: `.docx`, `.doc`
- **HTML**: `.html`, `.htm`
- **PPT**: `.pptx`, `.ppt`
- **图片**: `.png`, `.jpg`, `.jpeg`, `.bmp`, `.gif`, `.tiff`
- **表格**: `.csv`, `.xlsx`, `.xls`

### 支持的转换
- PDF ↔ Word, HTML, 图片, CSV
- Word ↔ PDF, HTML, CSV
- HTML ↔ PDF, Word
- PPT → PDF (文本模式)
- 图片 ↔ PDF, HTML
- CSV ↔ PDF, HTML, Excel

## 安装依赖

```bash
# 安装所有依赖
pip install -r requirements.txt

# 或者手动安装
pip install pandas openpyxl python-docx pillow
pip install PyPDF2 pdf2docx pdfplumber reportlab fpdf2
pip install python-pptx beautifulsoup4 html5lib img2pdf pdf2image
pip install Cython numpy setuptools
```

### 系统依赖

**Windows:**
- 下载并安装 [poppler](https://github.com/oschwartz10612/poppler-windows/releases/) 用于PDF转图片
- 可选: 安装 Microsoft Word 用于更好的Word转PDF效果

**Linux:**
```bash
sudo apt-get install poppler-utils libreoffice
```

## 使用方法

### Python直接使用

```python
from universal_converter import UniversalConverter

# 创建转换器
converter = UniversalConverter()

# 单文件转换
success = converter.convert("input.docx", "output.pdf")

# 批量转换
input_files = ["file1.pdf", "file2.docx"]
results = converter.convert_batch(input_files, "output_dir", "pdf")
```

### 命令行使用

```bash
# 单文件转换
python converter_cli.py -i input.docx -o output.pdf

# 指定格式转换
python converter_cli.py -i input.docx -f pdf

# 批量转换
python converter_cli.py -b file1.pdf file2.docx -d output_dir -f pdf

# 查看支持的格式
python converter_cli.py --list-formats
```

## 打包为DLL

### 1. 构建DLL

```bash
# 运行构建脚本
python build_dll.py

# 或者手动构建
python setup.py build_ext --inplace
```

构建完成后，DLL文件和相关文件会生成在 `release/` 目录中。

### 2. Qt项目集成

1. 包含头文件:
```cpp
#include "qt_converter.h"
```

2. 链接DLL文件到项目中

3. 使用转换器:
```cpp
QtFileConverter converter;

// 转换文件
if (converter.convertFile("input.docx", "output.pdf")) {
    qDebug() << "转换成功";
} else {
    qDebug() << "转换失败:" << converter.getLastError();
}

// 检查文件支持
if (converter.isFileSupported("test.pdf")) {
    qDebug() << "文件格式支持";
}

// 获取支持格式
QString formats = converter.getSupportedFormats();
```

## API文档

### UniversalConverter类

#### 主要方法
- `convert(input_path, output_path)` - 单文件转换
- `convert_batch(input_files, output_dir, output_format)` - 批量转换
- `validate_file(file_path)` - 验证文件
- `get_file_type(file_path)` - 获取文件类型
- `get_supported_conversions()` - 获取支持的转换

### QtFileConverter类

#### 主要方法
- `convertFile(inputPath, outputPath)` - 转换文件
- `isFileSupported(filePath)` - 检查文件支持
- `getFileType(filePath)` - 获取文件类型
- `getSupportedFormats()` - 获取支持格式
- `getLastError()` - 获取错误信息

## 测试

```bash
# 运行测试
python test_converter.py
```

## 注意事项

1. **内存使用**: 处理大文件时注意内存消耗
2. **依赖环境**: 某些转换功能需要系统依赖
3. **格式限制**: 复杂格式的转换可能有限制
4. **错误处理**: 建议在生产环境中添加适当的错误处理

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request！