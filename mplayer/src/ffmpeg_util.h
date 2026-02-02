#pragma once
#define _CRT_SECURE_NO_WARNINGS 1






// 全局配置
#define DEFAULT_BLOCK_TIMEOUT    10      // 阻塞超时时间（秒）
#define RTSP_TIMEOUT_US          5000000 // RTSP超时（微秒）
#define BUFFER_SIZE              2048000 * 10 // 缓冲区大小
#define DEFAULT_DST_PIX_FMT      AV_PIX_FMT_YUV420P // 默认输出像素格式
#define SCALE_ALGORITHM          SWS_BICUBIC // 缩放算法

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstring>
#include <ctime>
#include <chrono>
#include <thread>
#include <conio.h> // 用于按键检测（退出）
#include <mutex>

#ifdef _WIN32
#include <guiddef.h>
#include <dshow.h>
#include <windows.h>
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")
#endif

extern "C" {
#include "libavutil/avutil.h"
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
}

// 缓冲区状态：保证解码/渲染线程互斥访问
enum BufferState {
    BUFFER_IDLE,    // 空闲（可写入）
    BUFFER_FILLED,  // 已填充（可读取）
    BUFFER_ERROR    // 错误
};

// 共享帧缓冲区：替代原有临时拷贝，解码/渲染共用
struct SharedFrameBuffer {
    std::vector<uint8_t> buf;    // 实际数据缓冲区（YUV420P/ARGB）
    int w = 0;                   // 帧宽度
    int h = 0;                   // 帧高度
    int bpp = 0;                 // 每像素字节数
    int type = 0;                // 格式类型（0:YUV420P,1:BGR）
    int64_t ts = 0;              // 时间戳
    BufferState state = BUFFER_IDLE; // 缓冲区状态
    std::mutex mtx;              // 线程安全互斥锁
};






// 媒体类型（与原MEDIA_TYPE一致）
typedef enum {
	MEDIA_TYPE_UNKNOWN = 0,
	MEDIA_TYPE_CAPTURE,  // 摄像头采集
	MEDIA_TYPE_FILE,     // 本地文件
	MEDIA_TYPE_NETWORK   // 网络流（RTSP/HTTP等）
} MediaType;

// 解码模式（与原decode_mode一致）
typedef enum {
	SOFTWARE_DECODE = 0,
	HARDWARE_DECODE_CUVID, // NVIDIA硬解
	HARDWARE_DECODE_QSV    // Intel硬解
} DecodeMode;

// 帧数据结构（替换原hframe，原生存储解码后数据）
typedef struct {
	int w, h;            // 帧宽高
	int bpp;             // 每像素位数
	int type;            // 像素格式类型
	int64_t ts;          // 时间戳（毫秒）
	std::vector<uint8_t> buf; // 帧数据缓冲区
} FrameData;

// 播放器核心上下文（替换原HFFPlayer，纯原生FFmpeg对象+基础状态）
typedef struct {
	// FFmpeg核心对象
	AVFormatContext* fmt_ctx;
	AVCodecContext* codec_ctx;
	AVPacket* packet;
	AVFrame* frame;
	SwsContext* sws_ctx;
	AVDictionary* fmt_opts;
	AVDictionary* codec_opts;
	AVBufferRef* hw_device_ctx; // 硬解设备上下文

	// 流信息
	int video_stream_index;
	int audio_stream_index;
	int subtitle_stream_index;
	AVRational video_time_base;
	int src_w, src_h;
	AVPixelFormat src_pix_fmt;
	
	uint8_t* data[4];
	int linesize[4];

	// 状态控制
	int quit;
	time_t block_starttime;
	int block_timeout;
	int fps;
	int64_t duration;    // 总时长（毫秒）
	int64_t start_time;  // 开始时间（毫秒）
	int eof;
	int error;
	DecodeMode decode_mode;
	DecodeMode real_decode_mode;

	// 媒体信息
	MediaType media_type;
	std::string media_src;
	FrameData hframe;

    SharedFrameBuffer shared_buf; // 跨线程共享帧缓冲区
} FFPlayerContext;


//#define logi(...)  do { printf("[INFO] " __VA_ARGS__); printf("\n"); } while(0)
//#define loge(...)  do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)
// 日志开关：1=调试模式（打印日志），0=发布模式（空实现，零耗时）
#define LOG_ENABLE 1

// 调试/发布兼容的日志宏：LOG_ENABLE=1时打印，LOG_ENABLE=0时无任何操作
#if LOG_ENABLE
#define logi(...)  do { printf("[INFO] " __VA_ARGS__); printf("\n"); fflush(stdout); } while(0)
#define loge(...)  do { printf("[ERROR] " __VA_ARGS__); printf("\n"); fflush(stdout); } while(0)
#else
// 发布模式：空宏，编译器会直接优化，零耗时、零IO、零CPU占用
#define logi(...)  do {} while(0)
#define loge(...)  do {} while(0)
#endif
