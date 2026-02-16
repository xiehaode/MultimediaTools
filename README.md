# MultiMediaTool

一个功能强大的综合多媒体处理工具集，集成视频处理、图像处理、文档转换和网络服务等多种功能。

## 📋 项目概述

MultiMediaTool 是一个专业的多媒体处理平台，采用模块化架构设计，支持多种媒体格式的处理和转换。项目集成了现代化的开发工具和最佳实践，提供稳定可靠的多媒体处理服务。

### ✨ 主要特性

- 🎥 **视频处理** - 格式转换、剪辑、合并、截图、缩略图生成
- 🖼️ **图像处理** - 多种滤镜效果、格式转换、批量处理、水印添加
- 📄 **文档转换** - PDF、Word、HTML、PPT等格式互转，支持批量转换
- 🌐 **网络服务** - HTTP RESTful API、用户认证、文件管理
- 🎵 **媒体播放** - 多格式播放、流媒体支持
- 🛠️ **开发友好** - 模块化设计、完善文档、易于扩展

## 🏗️ 项目架构

```
MultiMediaTool/
├── MultiMediaTool/          # 主应用程序 (Qt/C++)
├── OpenCVTools/             # 图像和视频处理核心 (OpenCV/FFmpeg)
├── mediaServer/             # HTTP服务器 (C++17/cpprestsdk)
├── WordToPdf/               # 文档格式转换 (Python)
├── formatChange/            # 音视频格式转换 (FFmpeg)
├── mplayer/                 # 媒体播放器 (FFmpeg/SDL2)
├── curlAli/                 # 网络通信 (libcurl)
├── MultiMediatoolTest/      # 测试项目
├── MultiMediaTool.sln        # Visual Studio解决方案
├── configure.sh              # 配置脚本
├── generate_version.sh       # 版本生成脚本
└── pack.iss                  # 打包脚本
```

## 🚀 快速开始

### 系统要求

- **操作系统**: Windows 10/11 (x64)
- **开发环境**: Visual Studio 2017+ / Qt 5.12+
- **运行时**: .NET Framework 4.7.2+

### 安装依赖

#### 核心依赖
```bash
# Qt框架
# 下载并安装 Qt 5.12.11 或更高版本

# OpenCV
# 下载并配置 OpenCV 4.x

# FFmpeg
# 下载并配置 FFmpeg 4.x 或更高版本

# MySQL (可选，用于mediaServer)
# 安装 MySQL 8.0+
```

#### Python依赖 (WordToPdf模块)
```bash
pip install python-docx PyPDF2 pdf2image Pillow reportlab
```

### 构建项目

#### 使用Visual Studio
```bash
# 打开解决方案文件
MultiMediaTool.sln

# 选择配置 (Release/Debug) 和平台 (x64)
# 生成解决方案
```

#### 使用qmake (Qt项目)
```bash
cd MultiMediaTool
qmake MultiMediaTool.pro
make  # Windows上使用 nmake 或 mingw32-make
```

#### 使用CMake (mediaServer)
```bash
cd mediaServer
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 运行应用

```bash
# 运行主应用程序
./bin/MultiMediaTool.exe

# 运行HTTP服务器 (可选)
./mediaServer/bin/mediaServer.exe
```

## 📖 功能模块详解

### 🎥 视频处理模块 (OpenCVTools)

**核心功能:**
- 视频格式转换 (MP4, AVI, MKV, MOV等)
- 视频剪辑和合并
- 视频截图和缩略图生成
- RTSP流媒体支持
- 视频编码解码优化

**API示例:**
```cpp
#include "OpenCVFFMpegTools.h"

// 视频转换
VideoConverter converter;
converter.convert("input.mp4", "output.avi", "avi");

// 生成缩略图
ThumbnailGenerator::generate("video.mp4", "thumbnail.jpg", 0, 320, 240);
```

### 🖼️ 图像处理模块

**滤镜效果:**
- 灰度处理
- 油画效果
- 马赛克处理
- 磨皮美颜
- 亮度对比度调整
- 水印添加

**使用示例:**
```cpp
#include "CvTranslator.h"

// 应用滤镜
ImageFilter filter;
filter.applyGrayscale("input.jpg", "gray.jpg");
filter.applyOilPainting("input.jpg", "oil.jpg");
```

### 📄 文档转换模块 (WordToPdf)

**支持格式:**
- PDF ↔ Word (.docx)
- PDF ↔ HTML
- PPT → PDF
- 图片 ↔ PDF
- CSV 数据处理

**Python API:**
```python
from universal_converter import UniversalConverter

converter = UniversalConverter()
converter.pdf_to_word("input.pdf", "output.docx")
converter.word_to_pdf("input.docx", "output.pdf")
```

### 🌐 HTTP服务器模块 (mediaServer)

**RESTful API:**
```
POST /api/video/convert    # 视频转换
POST /api/image/filter      # 图像滤镜
POST /api/document/convert  # 文档转换
GET  /api/file/download/{id} # 文件下载
POST /api/auth/login        # 用户认证
```

**功能特性:**
- JWT认证
- 文件上传下载
- 任务队列管理
- 实时状态监控

## 🛠️ 开发指南



### 调试配置

```cpp
// 启用调试模式
#define DEBUG_MODE 1

// 崩溃转储
EnableCrashDumps();

// 日志输出
spdlog::info("Application started");
```


## 📦 部署和分发

### 自动打包

```bash
# 使用Inno Setup打包
iscc pack.iss

# 输出安装包
# Output/MultiMediaTool-Setup.exe
```

### 版本管理

```bash
# 自动生成版本号
./generate_version.sh

# 版本格式: v1.2.3-build{commits}-{hash}
```

### 部署结构

```
MultiMediaTool/
├── MultiMediaTool.exe          # 主程序
├── mediaServer.exe            # 服务器 (可选)
├── QtCore.dll                # Qt依赖
├── QtGui.dll                 # Qt依赖
├── opencv_worldxx.dll        # OpenCV依赖
├── ffmpeg.exe                 # FFmpeg可执行文件
├── config/                    # 配置文件
├── plugins/                   # 插件目录
└── temp/                      # 临时文件目录
```

## 🤝 贡献指南

我们欢迎社区贡献！请遵循以下步骤：

1. **Fork** 项目到您的GitHub账户
2. **创建** 功能分支: `git checkout -b feature/amazing-feature`
3. **提交** 您的更改: `git commit -m 'Add amazing feature'`
4. **推送** 到分支: `git push origin feature/amazing-feature`
5. **创建** Pull Request



## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。


---

**MultiMediaTool** - 让多媒体处理变得简单高效！

如果这个项目对您有帮助，请给我们一个 ⭐️ Star！