#pragma once  
#include "pch.h"
#include "OpenCVFFMpegTools.h"
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVFrame;

// 视频封装器上下文结构体
typedef struct {
	
	AVFormatContext *fmt_ctx;    // 封装格式上下文
	AVCodecContext  *codec_ctx;  // 编码上下文
	AVStream        *video_stream;// 视频流
	
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
	FFmpegEncoder();  
	
	int video_muxer_create(const char *output_path, int width, int height, int fps);
	int video_muxer_write_frame(AVFrame *frame, int frame_idx);
	int video_muxer_flush();
	void video_muxer_destroy();

private:
	
	int video_muxer_init_codec();
	void video_muxer_set_defaults();
	
	VideoMuxerCtx* mCtx = nullptr;  // C++11及以上直接初始化，避免野指针
};