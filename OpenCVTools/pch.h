// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H
#define _CRT_SECURE_NO_WARNINGS 1
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
// 添加要在此处预编译的标头
#include "framework.h"
#include <cerrno>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>


// Windows摄像头依赖（dshow）
#ifdef _WIN32
#include <guiddef.h>
#include <dshow.h>
#include <windows.h>
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")
#endif







extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
#include <libavutil/frame.h>
#include <libavutil/timestamp.h>
#include <libavutil/mem.h>
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"
}







// 全局配置（替换原配置文件g_confile，默认值与原代码一致）
#define DEFAULT_BLOCK_TIMEOUT    10      // 阻塞超时时间（秒）
#define RTSP_TIMEOUT_US          5000000 // RTSP超时（微秒）
#define BUFFER_SIZE              2048000 // 缓冲区大小
#define DEFAULT_DST_PIX_FMT      AV_PIX_FMT_YUV420P // 默认输出像素格式
#define SCALE_ALGORITHM          SWS_BICUBIC // 缩放算法


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

// FFPlayerContext 定义已移动到 pch.h，避免重定义



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

// FFPlayerContext 单一定义，避免重定义
typedef struct FFPlayerContext {
	// FFmpeg核心对象
	AVFormatContext* fmt_ctx;
	AVCodecContext* codec_ctx;
	AVCodecContext* video_decode_ctx;  // 兼容字段，指向 codec_ctx
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
	int video_index;  // 兼容字段，指向 video_stream_index
	AVRational video_time_base;
	int src_w, src_h;
	AVPixelFormat src_pix_fmt;
	int dst_w, dst_h;
	AVPixelFormat dst_pix_fmt;
	uint8_t* dst_data[4];
	int dst_linesize[4];
	uint8_t* data[4];  // 兼容字段，用于帧转换
	int linesize[4];   // 兼容字段，用于帧转换

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
	SharedFrameBuffer shared_buf;  // 共享帧缓冲区
	bool is_quit;
} FFPlayerContext;

// 日志封装（替换原hlogi/hloge，原生printf）
#define logi(...)  do { printf("[INFO] " __VA_ARGS__); printf("\n"); } while(0)
#define loge(...)  do { printf("[ERROR] " __VA_ARGS__); printf("\n"); } while(0)





#endif //PCH_H
