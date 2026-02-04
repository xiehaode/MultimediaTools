#include "pch.h"
#include "FFmpegEncoder.h"
// 构造函数：初始化上下文句柄为nullptr
FFmpegEncoder::FFmpegEncoder() : mCtx(nullptr)
{
	
}


void FFmpegEncoder::video_muxer_set_defaults()
{
	if (!mCtx) return; // 前置空指针校验
	mCtx->bit_rate = 4000000;  // 默认4Mbps码率
	mCtx->gop_size = 25;       // 默认25帧一个关键帧
	mCtx->fmt_ctx = nullptr;
	mCtx->codec_ctx = nullptr;
	mCtx->video_stream = nullptr;
	mCtx->is_init = 0; // 初始化为未初始化状态
}

// 初始化编码器和视频流（内部工具函数）
int FFmpegEncoder::video_muxer_init_codec()
{
	if (!mCtx)
	{
		av_log(NULL, AV_LOG_ERROR, "初始化编码器失败：上下文未分配\n");
		return -1;
	}

	AVCodec *codec = nullptr;
	int ret = 0;
	char err_buf[64] = { 0 };

	// 查找H.264编码器
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		av_log(NULL, AV_LOG_ERROR, "未找到H.264编码器，请确保FFmpeg编译了libx264\n");
		return -1;
	}

	// 创建视频流
	mCtx->video_stream = avformat_new_stream(mCtx->fmt_ctx, nullptr);
	if (!mCtx->video_stream)
	{
		av_log(NULL, AV_LOG_ERROR, "创建视频流失败\n");
		return -1;
	}
	// 设置流时间基和帧率
	mCtx->video_stream->time_base = av_make_q(1, mCtx->fps);
	mCtx->video_stream->r_frame_rate = av_make_q(mCtx->fps, 1);

	// 分配编码上下文
	mCtx->codec_ctx = avcodec_alloc_context3(codec);
	if (!mCtx->codec_ctx)
	{
		av_log(NULL, AV_LOG_ERROR, "分配编码上下文失败\n");
		return -1;
	}

	// 配置编码核心参数
	AVCodecContext *c = mCtx->codec_ctx;
	c->codec_id = AV_CODEC_ID_H264;
	c->codec_type = AVMEDIA_TYPE_VIDEO;
	c->pix_fmt = AV_PIX_FMT_YUV420P; // 必须YUV420P，兼容所有播放器
	c->width = mCtx->width;
	c->height = mCtx->height;
	c->time_base = mCtx->video_stream->time_base;
	c->framerate = mCtx->video_stream->r_frame_rate;
	c->bit_rate = mCtx->bit_rate;
	c->gop_size = mCtx->gop_size;
	c->max_b_frames = 0; // 关闭B帧，简化时间戳处理，提升兼容性

	// H.264编码选项：baseline profile提升兼容性，medium平衡速度/画质
	av_opt_set(c->priv_data, "profile", "baseline", 0);
	av_opt_set(c->priv_data, "preset", "medium", 0);

	// 全局头设置（MP4/MKV等格式必需，避免裸流）
	if (mCtx->fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
	{
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	// 复制编码参数到视频流
	ret = avcodec_parameters_from_context(mCtx->video_stream->codecpar, c);
	if (ret < 0)
	{
		av_strerror(ret, err_buf, sizeof(err_buf));
		av_log(NULL, AV_LOG_ERROR, "复制编码参数到视频流失败：%s\n", err_buf);
		return ret;
	}

	// 打开编码器
	ret = avcodec_open2(c, codec, nullptr);
	if (ret < 0)
	{
		av_strerror(ret, err_buf, sizeof(err_buf));
		av_log(NULL, AV_LOG_ERROR, "打开H.264编码器失败：%s\n", err_buf);
		return ret;
	}

	return 0;
}


int FFmpegEncoder::video_muxer_create(const char *output_path, int width, int height, int fps)
{
	
	if (!output_path || width % 2 != 0 || height % 2 != 0 || fps <= 0)
	{
		av_log(NULL, AV_LOG_ERROR, "参数非法：路径为空/分辨率非偶数（当前%dx%d）/帧率<=0（当前%d）\n", width, height, fps);
		return -1; // 修正：int返回值不能用NULL
	}

	
	VideoMuxerCtx *ctx = (VideoMuxerCtx *)av_mallocz(sizeof(VideoMuxerCtx));
	if (!ctx)
	{
		av_log(NULL, AV_LOG_ERROR, "分配VideoMuxerCtx内存失败\n");
		return -1;
	}

	
	mCtx = ctx;
	mCtx->output_path = av_strdup(output_path); // 复制路径，避免外部指针失效
	mCtx->width = width;
	mCtx->height = height;
	mCtx->fps = fps;
	video_muxer_set_defaults(); // 现在mCtx已赋值，可安全调用

	int ret = -1;
	char err_buf[64] = { 0 };

	
	ret = avformat_alloc_output_context2(&mCtx->fmt_ctx, nullptr, nullptr, output_path);
	if (ret < 0)
	{
		av_strerror(ret, err_buf, sizeof(err_buf));
		av_log(NULL, AV_LOG_ERROR, "初始化封装格式上下文失败：%s\n", err_buf);
		goto fail;
	}

	
	ret = video_muxer_init_codec();
	if (ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "初始化编码器失败，错误码：%d\n", ret);
		goto fail;
	}

	
	if (!(mCtx->fmt_ctx->oformat->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&mCtx->fmt_ctx->pb, output_path, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			av_strerror(ret, err_buf, sizeof(err_buf));
			av_log(NULL, AV_LOG_ERROR, "打开输出文件失败：%s\n", err_buf);
			goto fail;
		}
	}

	
	ret = avformat_write_header(mCtx->fmt_ctx, nullptr);
	if (ret < 0)
	{
		av_strerror(ret, err_buf, sizeof(err_buf));
		av_log(NULL, AV_LOG_ERROR, "写入文件头失败：%s\n", err_buf);
		goto fail;
	}

	
	mCtx->is_init = 1;
	av_log(NULL, AV_LOG_INFO, "视频封装器初始化成功：%s（%dx%d，%dfps，%dMbps）\n",
		output_path, width, height, fps, mCtx->bit_rate / 1000000);
	return 0; // 修正：统一返回0表示成功


fail:
	if (mCtx)
	{
		if (mCtx->output_path) av_free((void *)mCtx->output_path);
		if (mCtx->fmt_ctx)
		{
			if (!(mCtx->fmt_ctx->oformat->flags & AVFMT_NOFILE))
			{
				avio_close(mCtx->fmt_ctx->pb);
			}
			avformat_free_context(mCtx->fmt_ctx);
		}
		if (mCtx->codec_ctx) avcodec_free_context(&mCtx->codec_ctx);
		av_free(mCtx);
		mCtx = nullptr; // 置空，避免野指针
	}
	return -1;
}

// 写入单个AVFrame
int FFmpegEncoder::video_muxer_write_frame(AVFrame *frame, int frame_idx)
{
	if (!mCtx || !mCtx->is_init || !frame || frame_idx < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "写入帧失败：上下文未初始化/帧为空/索引非法（当前%d）\n", frame_idx);
		return -1;
	}

	// 校验帧格式和分辨率
	if (frame->format != AV_PIX_FMT_YUV420P || frame->width != mCtx->width || frame->height != mCtx->height)
	{
		av_log(NULL, AV_LOG_ERROR, "帧参数不匹配：要求YUV420P/%dx%d，实际格式%d/%dx%d\n",
			mCtx->width, mCtx->height, frame->format, frame->width, frame->height);
		return -1;
	}

	int ret = 0;
	AVPacket pkt = { 0 };
	char err_buf[64] = { 0 };

	// 设置帧PTS时间戳,基于编码时间基 1/fps，直接用帧索引
	frame->pts = frame_idx;
	frame->pict_type = AV_PICTURE_TYPE_NONE; // 由编码器自动判断帧类型（I/P帧）

	// 发送帧到编码器
	ret = avcodec_send_frame(mCtx->codec_ctx, frame);
	if (ret < 0)
	{
		av_strerror(ret, err_buf, sizeof(err_buf));
		av_log(NULL, AV_LOG_ERROR, "发送帧到编码器失败：%s\n", err_buf);
		av_packet_unref(&pkt);
		return ret;
	}

	// 循环接收编码器输出的AVPacket,一帧可能对应多个包，或多个帧对应一个包
	while (ret >= 0)
	{
		ret = avcodec_receive_packet(mCtx->codec_ctx, &pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			break; // 无更多包，退出循环
		}
		else if (ret < 0)
		{
			av_strerror(ret, err_buf, sizeof(err_buf));
			av_log(NULL, AV_LOG_ERROR, "接收编码器包失败：%s\n", err_buf);
			av_packet_unref(&pkt);
			return ret;
		}

		// 修正Packet时间戳（编码时间基 → 流时间基），关联流索引
		av_packet_rescale_ts(&pkt, mCtx->codec_ctx->time_base, mCtx->video_stream->time_base);
		pkt.stream_index = mCtx->video_stream->index;

		// 写入封装上下文,av_interleaved_write_frame：交错写入，支持后续扩展音频）
		ret = av_interleaved_write_frame(mCtx->fmt_ctx, &pkt);
		if (ret < 0)
		{
			av_strerror(ret, err_buf, sizeof(err_buf));
			av_log(NULL, AV_LOG_ERROR, "写入帧到封装器失败：%s\n", err_buf);
			av_packet_unref(&pkt);
			return ret;
		}

		av_packet_unref(&pkt); // 释放Packet资源，避免内存泄漏（关键）
	}

	return 0;
}

// 刷新编码器缓存并写入文件尾
int FFmpegEncoder::video_muxer_flush()
{
	if (!mCtx || !mCtx->is_init)
	{
		av_log(NULL, AV_LOG_ERROR, "刷新编码器失败：上下文未初始化\n");
		return -1;
	}

	int ret = 0;
	AVPacket pkt = { 0 };
	char err_buf[64] = { 0 };

	// 发送NULL触发编码器缓存刷新（输出所有未编码的帧，关键：避免视频尾部丢失）
	ret = avcodec_send_frame(mCtx->codec_ctx, nullptr);
	if (ret < 0 && ret != AVERROR_EOF)
	{
		av_strerror(ret, err_buf, sizeof(err_buf));
		av_log(NULL, AV_LOG_ERROR, "触发编码器刷新失败：%s\n", err_buf);
		av_packet_unref(&pkt);
		return ret;
	}

	// 接收并写入所有缓存的Packet
	while (ret >= 0)
	{
		ret = avcodec_receive_packet(mCtx->codec_ctx, &pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			break;
		}
		else if (ret < 0)
		{
			av_strerror(ret, err_buf, sizeof(err_buf));
			av_log(NULL, AV_LOG_ERROR, "接收刷新的编码器包失败：%s\n", err_buf);
			av_packet_unref(&pkt);
			return ret;
		}

		av_packet_rescale_ts(&pkt, mCtx->codec_ctx->time_base, mCtx->video_stream->time_base);
		pkt.stream_index = mCtx->video_stream->index;

		ret = av_interleaved_write_frame(mCtx->fmt_ctx, &pkt);
		if (ret < 0)
		{
			av_strerror(ret, err_buf, sizeof(err_buf));
			av_log(NULL, AV_LOG_ERROR, "写入刷新的帧失败：%s\n", err_buf);
			av_packet_unref(&pkt);
			return ret;
		}

		av_packet_unref(&pkt);
	}

	// 写入文件尾
	av_write_trailer(mCtx->fmt_ctx);
	av_log(NULL, AV_LOG_INFO, "视频封装收尾完成，文件尾已写入：%s\n", mCtx->output_path);
	return 0;
}

// 销毁实例并释放所有资源,逆序释放 
void FFmpegEncoder::video_muxer_destroy()
{
	if (!mCtx) return;

	VideoMuxerCtx *c = mCtx;
	// 逆序释放FFmpeg资源：编码器 → 封装器 → 文件句柄 → 路径 → 上下文
	if (c->codec_ctx)
	{
		avcodec_free_context(&c->codec_ctx);
		c->codec_ctx = nullptr;
	}
	if (c->fmt_ctx)
	{
		if (!(c->fmt_ctx->oformat->flags & AVFMT_NOFILE))
		{
			avio_close(c->fmt_ctx->pb); // 关闭文件句柄，避免资源泄漏
		}
		avformat_free_context(c->fmt_ctx);
		c->fmt_ctx = nullptr;
	}
	if (c->output_path)
	{
		av_free((void *)c->output_path); // 释放av_strdup复制的路径
		c->output_path = nullptr;
	}
	// 释放上下文结构体本身并置空类成员，避免野指针
	av_free(c);
	mCtx = nullptr;

	av_log(NULL, AV_LOG_INFO, "视频封装器已销毁，所有资源释放完成\n");
}

extern "C" OPENCVFFMPEGTOOLS_API void* Encoder_Create()
{
	return new FFmpegEncoder();
}