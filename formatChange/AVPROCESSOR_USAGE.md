# AVProcessor 接口使用说明

## 概述
AVProcessor 是一个高级音视频处理器，提供了丰富的音视频处理功能，包括格式转换、视频编辑、滤镜应用、水印添加等。

## C++ 接口

### 类定义
```cpp
class AVProcessor {
public:
    AVProcessor();
    ~AVProcessor();

    // 初始化处理器
    bool Initialize();
    
    // 设置回调函数
    void SetProgressCallback(ProgressCallback callback);
    void SetErrorCallback(ErrorCallback callback);
    
    // 视频处理功能
    bool ConvertVideo(const std::string& inputFile, const std::string& outputFile, 
                     const std::string& format, int width = -1, int height = -1, int fps = -1);
    
    bool ResizeVideo(const std::string& inputFile, const std::string& outputFile, 
                    int width, int height);
    
    bool ExtractVideoFrame(const std::string& inputFile, const std::string& outputFile, 
                           double timestamp = 0.0);
    
    bool SplitVideo(const std::string& inputFile, const std::string& outputFile, 
                    double startTime, double duration);
    
    bool MergeVideos(const std::string* inputFiles, int fileCount, const std::string& outputFile);
    
    // 音频处理功能
    bool ConvertAudio(const std::string& inputFile, const std::string& outputFile, 
                     const std::string& format, int sampleRate = -1, int channels = -1);
    
    bool ExtractAudioFromVideo(const std::string& inputFile, const std::string& outputFile);
    
    bool MergeAudioWithVideo(const std::string& videoFile, const std::string& audioFile, 
                             const std::string& outputFile);
    
    // 滤镜和特效
    bool ApplyVideoFilter(const std::string& inputFile, const std::string& outputFile, 
                         const std::string& filterGraph);
    
    bool AddWatermark(const std::string& inputFile, const std::string& outputFile, 
                     const std::string& watermarkImage, int x = 10, int y = 10);
    
    // 信息获取
    bool GetMediaInfo(const std::string& inputFile, 
                     int& width, int& height, double& duration, int& fps);
    
    // 进度和控制
    int GetProgress() const;
    void Cancel();
    void Pause();
    void Resume();
    
    // 错误处理
    std::string GetLastError() const;
    int GetLastErrorCode() const;
    
    // 批处理操作
    bool StartBatchProcess(const std::string* inputFiles, const std::string* outputFiles, 
                          int fileCount, const std::string& operation);
    bool IsBatchProcessComplete() const;
};
```

### 使用示例
```cpp
#include "AVProcessor.h"

// 回调函数
void ProgressCallback(int progress, const char* message) {
    std::cout << "进度: " << progress << "% - " << message << std::endl;
}

void ErrorCallback(int error_code, const char* error_message) {
    std::cout << "错误: " << error_message << std::endl;
}

int main() {
    AVProcessor processor;
    
    // 设置回调
    processor.SetProgressCallback(ProgressCallback);
    processor.SetErrorCallback(ErrorCallback);
    
    // 初始化
    if (!processor.Initialize()) {
        std::cout << "初始化失败: " << processor.GetLastError() << std::endl;
        return -1;
    }
    
    // 转换视频
    processor.ConvertVideo("input.mp4", "output.avi", "avi", 1280, 720, 30);
    
    // 添加水印
    processor.AddWatermark("input.mp4", "watermarked.mp4", "logo.png", 50, 50);
    
    return 0;
}
```

## C 接口

### 句柄类型定义
```c
typedef void* AVProcessorHandle;
typedef void(CALLING_CONVENTION* AVProcessorProgressCallback)(int progress, const char* message);
typedef void(CALLING_CONVENTION* AVProcessorErrorCallback)(int error_code, const char* error_message);
```

### 主要函数
```c
// 生命周期管理
AVProcessorHandle AVProcessor_Create();
void AVProcessor_Destroy(AVProcessorHandle handle);
int AVProcessor_Initialize(AVProcessorHandle handle);

// 回调设置
void AVProcessor_SetProgressCallback(AVProcessorHandle handle, AVProcessorProgressCallback callback);
void AVProcessor_SetErrorCallback(AVProcessorHandle handle, AVProcessorErrorCallback callback);

// 视频处理
int AVProcessor_ConvertVideo(AVProcessorHandle handle, const char* inputFile, 
                           const char* outputFile, const char* format, 
                           int width, int height, int fps);

int AVProcessor_ResizeVideo(AVProcessorHandle handle, const char* inputFile, 
                           const char* outputFile, int width, int height);

int AVProcessor_ExtractVideoFrame(AVProcessorHandle handle, const char* inputFile, 
                                const char* outputFile, double timestamp);

int AVProcessor_SplitVideo(AVProcessorHandle handle, const char* inputFile, 
                          const char* outputFile, double startTime, double duration);

int AVProcessor_MergeVideos(AVProcessorHandle handle, const char** inputFiles, 
                           int fileCount, const char* outputFile);

// 音频处理
int AVProcessor_ConvertAudio(AVProcessorHandle handle, const char* inputFile, 
                           const char* outputFile, const char* format, 
                           int sampleRate, int channels);

int AVProcessor_ExtractAudioFromVideo(AVProcessorHandle handle, const char* inputFile, 
                                     const char* outputFile);

int AVProcessor_MergeAudioWithVideo(AVProcessorHandle handle, const char* videoFile, 
                                   const char* audioFile, const char* outputFile);

// 滤镜和特效
int AVProcessor_ApplyVideoFilter(AVProcessorHandle handle, const char* inputFile, 
                                const char* outputFile, const char* filterGraph);

int AVProcessor_AddWatermark(AVProcessorHandle handle, const char* inputFile, 
                            const char* outputFile, const char* watermarkImage, int x, int y);

// 信息获取
int AVProcessor_GetMediaInfo(AVProcessorHandle handle, const char* inputFile, 
                           int* width, int* height, double* duration, int* fps);

// 进度和控制
int AVProcessor_GetProgress(AVProcessorHandle handle);
void AVProcessor_Cancel(AVProcessorHandle handle);
void AVProcessor_Pause(AVProcessorHandle handle);
void AVProcessor_Resume(AVProcessorHandle handle);

// 错误处理
int AVProcessor_GetLastErrorCode(AVProcessorHandle handle);
int AVProcessor_GetLastError(AVProcessorHandle handle, char* buffer, int bufferSize);

// 批处理操作
int AVProcessor_StartBatchProcess(AVProcessorHandle handle, const char** inputFiles, 
                                 const char** outputFiles, int fileCount, const char* operation);

int AVProcessor_IsBatchProcessComplete(AVProcessorHandle handle);
```

### C 接口使用示例
```c
#include <stdio.h>
#include "formatChange.h"

void ProgressCallback(int progress, const char* message) {
    printf("进度: %d%% - %s\n", progress, message);
}

void ErrorCallback(int error_code, const char* error_message) {
    printf("错误 [%d]: %s\n", error_code, error_message);
}

int main() {
    // 创建处理器
    AVProcessorHandle handle = AVProcessor_Create();
    if (!handle) {
        printf("创建失败!\n");
        return -1;
    }
    
    // 设置回调
    AVProcessor_SetProgressCallback(handle, ProgressCallback);
    AVProcessor_SetErrorCallback(handle, ErrorCallback);
    
    // 初始化
    if (!AVProcessor_Initialize(handle)) {
        printf("初始化失败!\n");
        AVProcessor_Destroy(handle);
        return -1;
    }
    
    // 转换视频
    AVProcessor_ConvertVideo(handle, "input.mp4", "output.avi", "avi", 1280, 720, 30);
    
    // 添加水印
    AVProcessor_AddWatermark(handle, "input.mp4", "watermarked.mp4", "logo.png", 50, 50);
    
    // 清理
    AVProcessor_Destroy(handle);
    return 0;
}
```

## 功能特性

### 视频处理功能
- **格式转换**: 支持多种视频格式之间的转换
- **分辨率调整**: 改变视频的分辨率
- **帧提取**: 从视频中提取特定时间点的帧
- **视频分割**: 按时间段分割视频
- **视频合并**: 合并多个视频文件
- **滤镜应用**: 应用各种视频滤镜效果
- **水印添加**: 在视频上添加图片水印

### 音频处理功能
- **格式转换**: 支持多种音频格式之间的转换
- **参数调整**: 调整采样率、声道数等参数
- **音频提取**: 从视频文件中提取音频
- **音视频合并**: 将音频与视频文件合并

### 控制功能
- **进度监控**: 实时获取处理进度
- **暂停/恢复**: 支持操作的暂停和恢复
- **取消操作**: 可以随时取消正在进行的操作
- **错误处理**: 详细的错误信息和错误码
- **批处理**: 支持批量处理多个文件

### 回调机制
- **进度回调**: 实时报告处理进度和状态信息
- **错误回调**: 异步报告错误信息

## 支持的格式

### 视频格式
- MP4, AVI, MKV, MOV, WMV, FLV, WebM, etc.

### 音频格式
- MP3, WAV, FLAC, AAC, OGG, M4A, etc.

### 图像格式
- PNG, JPG, JPEG, BMP, TIFF, GIF, etc.

## 编译和使用

### 编译 DLL
```bash
# 使用 Visual Studio
msbuild formatChange.vcxproj /p:Configuration=Release /p:Platform=x64

# 或者使用其他支持的编译器
```

### 链接到项目
1. 将 `formatChange.h` 头文件包含到项目中
2. 链接 `FORMATCHANGE.lib` 库文件
3. 确保 `FORMATCHANGE.dll` 在运行时可以找到

### 测试程序
项目包含以下测试程序：
- `test_avprocessor_c.cpp`: C++ 接口测试程序
- `test_avprocessor_c_interface.c`: C 接口测试程序

运行测试：
```bash
.\test_avprocessor_c.exe
.\test_avprocessor_c_interface.exe
```

## 注意事项
1. 调用任何处理函数前必须先调用 `Initialize()`
2. 设置回调函数可以获得更好的用户体验
3. 使用 C 接口时，记得调用 `AVProcessor_Destroy()` 释放资源
4. 所有文件路径必须是有效的绝对路径或相对路径
5. 批处理操作适合处理大量文件，但不适合实时性要求高的场景
6. 暂停功能主要用于长时间的操作，短时间操作可能无法及时响应

## 错误代码说明
- `0`: 成功
- `-1`: 未知错误
- `-2`: 处理器未初始化或参数无效
- `-3`: 操作被用户取消
- `-4` 到 `-14`: 各种具体的处理错误

具体错误信息可以通过 `GetLastError()` 函数获取。