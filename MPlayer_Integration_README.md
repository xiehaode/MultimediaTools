# MPlayer跨进程调用集成

## 功能概述

已成功实现MultiMediaTool与MPlayer的跨进程调用和集成，支持：

1. **进程管理**：启动、停止、监控MPlayer进程
2. **参数传递**：支持命令行参数传递给MPlayer
3. **状态监控**：实时监控播放状态、进度信息
4. **错误处理**：完善的错误处理和用户反馈

## 集成架构

### 核心组件

1. **MPlayerManager** (`src/base/mplayermanager.h/cpp`)
   - 继承自QObject，基于QProcess
   - 提供完整的MPlayer进程管理功能
   - 支持异步命令通信

2. **VideoPage集成** (`src/gui/page/videopage.h/cpp`)
   - 在现有视频页面中集成MPlayer功能
   - 点击"开始"按钮启动MPlayer
   - 支持文件选择和播放控制

### 进程通信机制

```cpp
// 启动MPlayer进程
bool MPlayerManager::startMPlayer(const QString &filePath, const QStringList &args = QStringList());

// 发送控制命令
void MPlayerManager::sendCommand(const QString &command);

// 状态监控信号
void playbackStateChanged(PlaybackState state);
void positionChanged(qint64 position, qint64 duration);
void mediaFinished();
```

## 使用方法

### 1. 编译项目

```bash
# 首先编译MPlayer
cd mplayer
qmake && make

# 然后编译MultiMediaTool
cd ../MultiMediaTool
qmake && make
```

### 2. 启动应用

1. 运行MultiMediaTool.exe
2. 进入视频页面
3. 点击"开始"按钮
4. 系统会自动：
   - 检查MPlayer可执行文件是否存在
   - 弹出文件选择对话框
   - 选择视频文件后启动MPlayer播放

### 3. 支持的功能

- **文件格式**：MP4, AVI, MKV, FLV, MOV, WMV
- **播放控制**：播放、暂停、停止
- **进度跟踪**：实时显示播放进度
- **状态监控**：显示当前播放状态
- **错误处理**：友好的错误提示

## 技术实现

### MPlayer启动参数

```cpp
QStringList args;
args << "-quiet"           // 静默模式
      << "-slave"           // 从模式，接受命令
      << "-idle"           // 空闲时不退出
      << "-really-quiet"    // 完全静默
      << "-osdlevel" << "1" // 显示OSD
      << "-ontop"          // 置顶播放
      << "-monitorpixelaspect" << "1.0"; // 像素比例
```

### 路径管理

- Windows: `../../mplayer/debug/mplayer.exe` 或 `../../mplayer/release/mplayer.exe`
- Linux: `../../mplayer/mplayer`

### 状态枚举

```cpp
enum PlaybackState {
    Idle,       // 空闲
    Loading,    // 加载中
    Playing,    // 播放中
    Paused,     // 暂停
    Stopped,    // 停止
    Error       // 错误
};
```

## 扩展功能

### 添加新的控制命令

```cpp
// 播放控制
m_mplayerManager->sendCommand("pause");     // 暂停
m_mplayerManager->sendCommand("play");      // 播放
m_mplayerManager->sendCommand("stop");      // 停止
m_mplayerManager->sendCommand("seek 10");  // 跳转到10秒

// 音量控制
m_mplayerManager->sendCommand("volume 80"); // 设置音量为80%
```

### 自定义参数

```cpp
// 全屏播放
QStringList args;
args << "-fs" << "-zoom";
m_mplayerManager->startMPlayer(filePath, args);

// 指定音频输出
args << "-ao" << "alsa"; // Linux
// 或
args << "-ao" << "directsound"; // Windows
```

## 故障排除

### 1. MPlayer找不到
- 错误：`MPlayer is not available`
- 解决：确保先编译mplayer项目

### 2. 进程启动失败
- 检查mplayer可执行文件权限
- 确认路径配置正确

### 3. 通信失败
- 确保MPlayer编译时包含从模式支持
- 检查参数传递格式

## 文件结构

```
MultiMediaTool/
├── src/base/
│   ├── mplayermanager.h      # MPlayer管理器头文件
│   ├── mplayermanager.cpp    # MPlayer管理器实现
│   └── cmdexecutor.h       # 命令执行基类
├── src/gui/page/
│   ├── videopage.h         # 视频页面头文件（已修改）
│   └── videopage.cpp      # 视频页面实现（已修改）
└── MultiMediaTool.pro      # 项目文件（已修改）
```

## 总结

成功实现了：
- ✅ 跨进程调用MPlayer
- ✅ 参数传递和命令控制
- ✅ 实时状态监控
- ✅ 错误处理机制
- ✅ 用户友好的界面集成
- ✅ 完整的生命周期管理

这为MultiMediaTool提供了强大的视频播放能力，同时保持了良好的架构设计。