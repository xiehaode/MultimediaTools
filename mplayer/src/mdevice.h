#ifndef MDEVICE_H
#define MDEVICE_H

#include <vector>
#include <mutex>
#include <cstdint>

// 缓冲区状态：保证解码/渲染线程互斥访问
enum BufferState {
    BUFFER_IDLE,    // 空闲（可写入）
    BUFFER_FILLED,  // 已填充（可读取）
    BUFFER_ERROR    // 错误
};

// 共享帧缓冲区：解码/渲染共用
struct SharedFrameBuffer {
    std::vector<uint8_t> buf;    // 实际数据缓冲区
    int w = 0;                   // 帧宽度
    int h = 0;                   // 帧高度
    int bpp = 0;                 // 每像素字节数
    int type = 0;                // 格式类型（0:YUV420P, 1:BGR）
    int64_t ts = 0;              // 时间戳
    BufferState state = BUFFER_IDLE; // 状态标记
    std::mutex mtx;              // 互斥锁
};

struct HDevice {
    char name[256];
};

std::vector<HDevice> getVideoDevices();
std::vector<HDevice> getAudioDevices();

#endif // MDEVICE_H
