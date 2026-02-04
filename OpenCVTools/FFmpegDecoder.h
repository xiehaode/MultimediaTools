#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include "pch.h"
#include "OpenCVFFMpegTools.h"



class FFmpegDecoder
{
public:
	explicit FFmpegDecoder();
	~FFmpegDecoder();

	int ffplayer_open(const std::string &inputPath, bool isDevice = true);
	AVFrame *  getFrame();
	void ffplayer_read_frame();
	void ffplayer_close();

	SharedFrameBuffer* getSharedBuffer() { return &ctx->shared_buf; }
	int64_t getDuration() const;
	int64_t getCurrentTime() const;
	
	// 获取视频属性的方法
	int getWidth() const;
	int getHeight() const;
	int getFPS() const;
	AVCodecContext* getCodecContext() const { return ctx ? ctx->codec_ctx : nullptr; }
	AVFrame* getCurrentAVFrame() const { return ctx ? ctx->frame : nullptr; }



private:
	FFPlayerContext *ctx;
	bool m_paused = false;
	bool m_seek_req = false;
	int64_t m_seek_target = 0;
	std::mutex m_ctrl_mtx;


	int64_t m_frame_count = 0;
	std::mutex m_record_mtx;
};


#endif // PLAYER_H
