#include "pch.h"
#include "FFmpegDecoder.h"


FFmpegDecoder::FFmpegDecoder()
{
	avdevice_register_all();
	ctx = new FFPlayerContext();
}

FFmpegDecoder::~FFmpegDecoder()
{
	if (ctx->sws_ctx) sws_freeContext(ctx->sws_ctx);
	if (ctx->codec_ctx) avcodec_free_context(&ctx->codec_ctx);
	if (ctx->fmt_ctx) avformat_close_input(&ctx->fmt_ctx);
	if (ctx->frame) av_frame_free(&ctx->frame);
	delete ctx;
}

int FFmpegDecoder::ffplayer_open(const std::string &inputPath, bool isDevice)
{
	AVInputFormat *ifmt = nullptr;
	std::string path = inputPath;

	if (avformat_open_input(&ctx->fmt_ctx, path.c_str(), ifmt, nullptr) < 0) {
		return -1;
	}

	if (avformat_find_stream_info(ctx->fmt_ctx, nullptr) < 0) return -1;

	for (unsigned int i = 0; i < ctx->fmt_ctx->nb_streams; i++) {
		if (ctx->fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			ctx->video_stream_index = i;
			break;
		}
	}

	if (ctx->video_stream_index == -1) return -1;

	AVCodecParameters *codecpar = ctx->fmt_ctx->streams[ctx->video_stream_index]->codecpar;
	AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
	ctx->codec_ctx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(ctx->codec_ctx, codecpar);

	if (avcodec_open2(ctx->codec_ctx, codec, nullptr) < 0) return -1;

	ctx->frame = av_frame_alloc();
	return 0;
}

void FFmpegDecoder::ffplayer_read_frame()
{
	if (m_paused && !m_seek_req) {
		return;
	}

	{
		std::lock_guard<std::mutex> lock(m_ctrl_mtx);
		if (m_seek_req) {
			int64_t seek_target = av_rescale_q(m_seek_target * 1000, { 1, AV_TIME_BASE }, ctx->fmt_ctx->streams[ctx->video_stream_index]->time_base);
			if (av_seek_frame(ctx->fmt_ctx, ctx->video_stream_index, seek_target, AVSEEK_FLAG_BACKWARD) >= 0) {
				avcodec_flush_buffers(ctx->codec_ctx);
			}
			m_seek_req = false;
		}
	}

	AVPacket pkt = {0};
	//av_init_packet(&pkt);
	pkt.data = nullptr;
	pkt.size = 0;
	if (av_read_frame(ctx->fmt_ctx, &pkt) >= 0) {


		if (pkt.stream_index == ctx->video_stream_index) {
			if (avcodec_send_packet(ctx->codec_ctx, &pkt) == 0) {
				while (avcodec_receive_frame(ctx->codec_ctx, ctx->frame) == 0) {

					int dw = ctx->codec_ctx->width;
					int dh = ctx->codec_ctx->height;

					{
						std::lock_guard<std::mutex> lock(ctx->shared_buf.mtx);

						// 零拷贝准备：确保缓冲区大小正确
						int target_size = dw * dh * 4; // 假设转为 RGBA
						if (ctx->shared_buf.buf.size() != (size_t)target_size) {
							ctx->shared_buf.buf.resize(target_size);
							ctx->shared_buf.w = dw;
							ctx->shared_buf.h = dh;
							ctx->shared_buf.type = 1; // 标记为 RGBA/BGR
						}

						// 初始化 SwsContext
						if (!ctx->sws_ctx) {
							ctx->sws_ctx = sws_getContext(dw, dh, ctx->codec_ctx->pix_fmt,
								dw, dh, AV_PIX_FMT_RGBA,
								SWS_BILINEAR, nullptr, nullptr, nullptr);
						}

						// 直接写入共享缓冲区
						ctx->data[0] = ctx->shared_buf.buf.data();
						ctx->linesize[0] = dw * 4;

						sws_scale(ctx->sws_ctx, ctx->frame->data, ctx->frame->linesize,
							0, dh, ctx->data, ctx->linesize);

						ctx->shared_buf.ts = av_gettime_relative() / 1000;
						ctx->shared_buf.state = BUFFER_FILLED;
					}
				}
			}

		}
		av_packet_unref(&pkt);
	}
}

int FFmpegDecoder::read_frame_for_trans()
{
	{
		std::lock_guard<std::mutex> lock(m_ctrl_mtx);
		if (m_seek_req) {
			int64_t seek_target = av_rescale_q(m_seek_target * 1000, { 1, AV_TIME_BASE }, ctx->fmt_ctx->streams[ctx->video_stream_index]->time_base);
			if (av_seek_frame(ctx->fmt_ctx, ctx->video_stream_index, seek_target, AVSEEK_FLAG_BACKWARD) >= 0) {
				avcodec_flush_buffers(ctx->codec_ctx);
			}
			m_seek_req = false;
		}
	}

	AVPacket pkt = {0};
	//av_init_packet(&pkt);
	pkt.data = nullptr;
	pkt.size = 0;
	while (av_read_frame(ctx->fmt_ctx, &pkt) >= 0) {
		if (pkt.stream_index != ctx->video_stream_index) {
			av_packet_unref(&pkt);
			continue; // 跳过非视频流
		}

		int sendRet = avcodec_send_packet(ctx->codec_ctx, &pkt);
		if (sendRet == 0) {
			while (true) {
				int ret = avcodec_receive_frame(ctx->codec_ctx, ctx->frame);
				if (ret == 0) {
					av_packet_unref(&pkt);
					return 1; // 成功读取到一帧
				}
				if (ret == AVERROR(EAGAIN)) {
					break; // 需要更多数据包
				}
				av_packet_unref(&pkt);
				return -1; // 解码失败
			}
			av_packet_unref(&pkt);
			continue;
		}

		if (sendRet == AVERROR(EAGAIN)) {
			int ret = avcodec_receive_frame(ctx->codec_ctx, ctx->frame);
			if (ret == 0) {
				av_packet_unref(&pkt);
				return 1;
			}
			av_packet_unref(&pkt);
			continue; // 继续读取更多包
		}

		av_packet_unref(&pkt);
		return -1; // 送入解码器失败
	}



	// 尝试刷新解码器，读取缓冲区中可能剩余的帧
	if (avcodec_send_packet(ctx->codec_ctx, nullptr) == 0) {
		int ret = avcodec_receive_frame(ctx->codec_ctx, ctx->frame);
		if (ret == 0) {
			return 1;
		}
	}

	return 0; // EOF
}

void FFmpegDecoder::ffplayer_close()
{

}

int64_t FFmpegDecoder::getDuration() const
{
	if (ctx->fmt_ctx && ctx->fmt_ctx->duration != AV_NOPTS_VALUE) {
		return ctx->fmt_ctx->duration / (AV_TIME_BASE / 1000);
	}
	return 0;
}

int64_t FFmpegDecoder::getCurrentTime() const
{
	
	return 0;
}

int FFmpegDecoder::getWidth() const
{
	if (ctx && ctx->codec_ctx) {
		return ctx->codec_ctx->width;
	}
	return 0;
}

int FFmpegDecoder::getHeight() const
{
	if (ctx && ctx->codec_ctx) {
		return ctx->codec_ctx->height;
	}
	return 0;
}

int FFmpegDecoder::getFPS() const
{
	if (ctx && ctx->fmt_ctx && ctx->video_stream_index >= 0) {
		AVStream* video_stream = ctx->fmt_ctx->streams[ctx->video_stream_index];
		if (video_stream) {
			// 计算帧率
			AVRational fps = av_guess_frame_rate(ctx->fmt_ctx, video_stream, nullptr);
			if (fps.num && fps.den) {
				return fps.num / fps.den;
			}
		}
	}
	return 30; // 默认帧率
}

// C接口实现
extern "C" OPENCVFFMPEGTOOLS_API void* Decoder_Create()
{
	return new FFmpegDecoder();
}

extern "C" OPENCVFFMPEGTOOLS_API void Decoder_Destroy(void* decoder)
{
	if (decoder) {
		delete static_cast<FFmpegDecoder*>(decoder);
	}
}

extern "C" OPENCVFFMPEGTOOLS_API int Decoder_FFPlayerOpen(void* decoder, const char* input_path, int is_device)
{
	if (!decoder || !input_path) return -1;
	bool isDevice = (is_device != 0);
	return static_cast<FFmpegDecoder*>(decoder)->ffplayer_open(std::string(input_path), isDevice);
}

extern "C" OPENCVFFMPEGTOOLS_API void Decoder_FFPlayerReadFrame(void* decoder)
{
	if (!decoder) return;
	static_cast<FFmpegDecoder*>(decoder)->ffplayer_read_frame();
}

extern "C" OPENCVFFMPEGTOOLS_API void Decoder_FFPlayerClose(void* decoder)
{
	if (!decoder) return;
	static_cast<FFmpegDecoder*>(decoder)->ffplayer_close();
}

extern "C" OPENCVFFMPEGTOOLS_API int64_t Decoder_GetDuration(void* decoder)
{
	if (!decoder) return 0;
	return static_cast<FFmpegDecoder*>(decoder)->getDuration();
}

extern "C" OPENCVFFMPEGTOOLS_API int64_t Decoder_GetCurrentTime(void* decoder)
{
	if (!decoder) return 0;
	return static_cast<FFmpegDecoder*>(decoder)->getCurrentTime();
}

extern "C" OPENCVFFMPEGTOOLS_API int Decoder_GetWidth(void* decoder)
{
	if (!decoder) return 0;
	return static_cast<FFmpegDecoder*>(decoder)->getWidth();
}

extern "C" OPENCVFFMPEGTOOLS_API int Decoder_GetHeight(void* decoder)
{
	if (!decoder) return 0;
	return static_cast<FFmpegDecoder*>(decoder)->getHeight();
}

extern "C" OPENCVFFMPEGTOOLS_API int Decoder_GetFPS(void* decoder)
{
	if (!decoder) return 30;
	return static_cast<FFmpegDecoder*>(decoder)->getFPS();
}