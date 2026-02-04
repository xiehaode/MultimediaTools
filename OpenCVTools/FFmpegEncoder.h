#pragma once  // 必须放在文件最顶部，确保头文件只被包含一次
#include "pch.h"
#include "OpenCVFFMpegTools.h"
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVFrame;

// 视频封装器上下文结构体
typedef struct {
	// FFmpeg核心句柄
	AVFormatContext *fmt_ctx;    // 封装格式上下文
	AVCodecContext  *codec_ctx;  // 编码上下文
	AVStream        *video_stream;// 视频流
	// 配置参数
	const char      *output_path;// 输出视频路径
	int             width;       // 视频宽度（偶数）
	int             height;      // 视频高度（偶数）
	int             fps;         // 帧率
	int             bit_rate;    // 码率（默认4Mbps）
	int             gop_size;    // 关键帧间隔（默认25）
	// 状态标识
	int             is_init;     // 是否初始化完成（0-未初始化，1-已初始化）
} VideoMuxerCtx;

class FFmpegEncoder
{
public:
	FFmpegEncoder();  // 无参构造函数移除explicit
	// 对外暴露的核心方法
	int video_muxer_create(const char *output_path, int width, int height, int fps);
	int video_muxer_write_frame(AVFrame *frame, int frame_idx);
	int video_muxer_flush();
	void video_muxer_destroy();

private:
	// 内部工具方法，私有化避免外部调用
	int video_muxer_init_codec();
	void video_muxer_set_defaults();
	// 核心上下文句柄
	VideoMuxerCtx* mCtx = nullptr;  // C++11及以上直接初始化，避免野指针
};