#include "pch.h"
#include "FFmpegDecoder.h"
#include "OpenCVFFMpegTools.h"

// 静态成员初始化
std::atomic_flag FFmpegDecoder::s_ffmpeg_init = ATOMIC_FLAG_INIT;

// 中断回调实现
int FFmpegDecoder::interrupt_callback(void* opaque) {
	if (!opaque) return 0;
	FFmpegDecoder* decoder = static_cast<FFmpegDecoder*>(opaque);

	// 检查是否需要退出，或阻塞超时
	if (decoder->m_quit || (time(nullptr) - decoder->m_block_starttime > decoder->m_config.block_timeout)) {
		hlogi("interrupt callback: quit=%d, timeout=%d", decoder->m_quit.load(),
			(int)(time(nullptr) - decoder->m_block_starttime));
		return 1;
	}
	return 0;
}

// 构造函数
FFmpegDecoder::FFmpegDecoder() {
	// 全局初始化FFmpeg（仅第一次创建实例时执行）
	if (!s_ffmpeg_init.test_and_set()) {
		avformat_network_init();
		avdevice_register_all();
		hlogi("FFmpeg initialized");
	}

	// 初始化FFmpeg对象
	m_packet = av_packet_alloc();
	m_frame = av_frame_alloc();
}

// 析构函数
FFmpegDecoder::~FFmpegDecoder() {
	stop();
	close_input();
}

// 初始化播放器
int FFmpegDecoder::init(const PlayerConfig& config) {
	if (m_running) {
		hloge("Player is already running");
		return -1;
	}

	m_config = config;
	m_error_code = 0;
	m_eof = false;

	// 转换目标像素格式
	if (config.dst_format == PIX_FMT_YUV420P) {
		m_dst_pix_fmt = AV_PIX_FMT_YUV420P;
	}
	else if (config.dst_format == PIX_FMT_BGR24) {
		m_dst_pix_fmt = AV_PIX_FMT_BGR24;
	}
	else {
		hloge("Unsupported pixel format");
		return -2;
	}

	// 打开输入源
	int ret = open_input();
	if (ret != 0) {
		hloge("Open input failed: %d", ret);
		m_error_code = ret;
		return ret;
	}

	hlogi("Player initialized successfully: %s", config.src.c_str());
	hlogi("Video info: %dx%d, fps=%d, duration=%lldms",
		m_width, m_height, m_fps, m_duration);

	return 0;
}

// 打开输入源并初始化解码器
int FFmpegDecoder::open_input() {
	// 1. 构建输入URL
	std::string ifile;
	AVInputFormat* ifmt = nullptr;

	switch (m_config.type) {
	case MEDIA_TYPE_CAPTURE: {
		ifile = "video=" + m_config.src;
#ifdef _WIN32
		const char* drive = "dshow";
#elif defined(__linux__)
		const char* drive = "v4l2";
#else
		const char* drive = "avfoundation";
#endif
		ifmt = av_find_input_format(drive);
		if (!ifmt) {
			hloge("Cannot find input format: %s", drive);
			return -5;
		}
		break;
	}
	case MEDIA_TYPE_FILE:
	case MEDIA_TYPE_NETWORK:
		ifile = m_config.src;
		break;
	default:
		hloge("Unsupported media type");
		return -10;
	}

	// 2. 分配格式上下文
	m_fmt_ctx = avformat_alloc_context();
	if (!m_fmt_ctx) {
		hloge("avformat_alloc_context failed");
		return -11;
	}

	// 3. 设置输入参数
	if (m_config.type == MEDIA_TYPE_NETWORK) {
		// RTSP参数配置
		if (ifile.substr(0, 5) == "rtsp:") {
			av_dict_set(&m_fmt_opts, "rtsp_transport", "tcp", 0); // 默认TCP
			av_dict_set(&m_fmt_opts, "stimeout", "5000000", 0);   // 5秒超时
		}
	}
	av_dict_set(&m_fmt_opts, "buffer_size", "2048000", 0); // 缓冲区2MB

	// 4. 设置中断回调
	m_fmt_ctx->interrupt_callback.callback = interrupt_callback;
	m_fmt_ctx->interrupt_callback.opaque = this;
	m_block_starttime = time(nullptr);

	// 5. 打开输入
	int ret = avformat_open_input(&m_fmt_ctx, ifile.c_str(), ifmt, &m_fmt_opts);
	if (ret < 0) {
		hloge("avformat_open_input failed: %d", ret);
		avformat_free_context(m_fmt_ctx);
		m_fmt_ctx = nullptr;
		return ret;
	}

	// 6. 查找流信息
	ret = avformat_find_stream_info(m_fmt_ctx, nullptr);
	if (ret < 0) {
		hloge("avformat_find_stream_info failed: %d", ret);
		avformat_close_input(&m_fmt_ctx);
		m_fmt_ctx = nullptr;
		return ret;
	}

	// 7. 找到视频流
	m_video_stream_index = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (m_video_stream_index < 0) {
		hloge("Cannot find video stream");
		avformat_close_input(&m_fmt_ctx);
		m_fmt_ctx = nullptr;
		return -20;
	}

	AVStream* video_stream = m_fmt_ctx->streams[m_video_stream_index];
	m_video_time_base = video_stream->time_base;

	// 8. 获取解码器参数
	AVCodecParameters* codec_param = video_stream->codecpar;
	m_width = codec_param->width;
	m_height = codec_param->height;
	m_src_pix_fmt = (AVPixelFormat)codec_param->format;
	//m_src_pix_fmt = (AVPixelFormat)codec_param->pix_fmt;

	if (m_width <= 0 || m_height <= 0 || m_src_pix_fmt == AV_PIX_FMT_NONE) {
		hloge("Invalid video parameters");
		avformat_close_input(&m_fmt_ctx);
		m_fmt_ctx = nullptr;
		return -21;
	}

	// 9. 计算目标分辨率（4字节对齐）
	m_dst_width = m_width >> 2 << 2;
	m_dst_height = m_height;

	// 10. 获取解码器（优先硬件解码）
	AVCodec* codec = nullptr;
	DecodeMode real_decode_mode = SOFTWARE_DECODE;

	if (m_config.decode_mode != SOFTWARE_DECODE) {
		std::string decoder_name = avcodec_get_name(codec_param->codec_id);
		if (m_config.decode_mode == HARDWARE_DECODE_CUVID) {
			decoder_name += "_cuvid";
			real_decode_mode = HARDWARE_DECODE_CUVID;
		}
		else if (m_config.decode_mode == HARDWARE_DECODE_QSV) {
			decoder_name += "_qsv";
			real_decode_mode = HARDWARE_DECODE_QSV;
		}

		codec = avcodec_find_decoder_by_name(decoder_name.c_str());
		if (!codec) {
			hlogi("Hardware decoder %s not found, use software decoder", decoder_name.c_str());
		}
	}

	// 硬件解码失败则使用软件解码
	if (!codec) {
		codec = avcodec_find_decoder(codec_param->codec_id);
		if (!codec) {
			hloge("Cannot find decoder for codec ID: %d", codec_param->codec_id);
			avformat_close_input(&m_fmt_ctx);
			m_fmt_ctx = nullptr;
			return -30;
		}
	}

	// 11. 初始化解码器上下文
	m_codec_ctx = avcodec_alloc_context3(codec);
	if (!m_codec_ctx) {
		hloge("avcodec_alloc_context3 failed");
		avformat_close_input(&m_fmt_ctx);
		m_fmt_ctx = nullptr;
		return -31;
	}

	ret = avcodec_parameters_to_context(m_codec_ctx, codec_param);
	if (ret < 0) {
		hloge("avcodec_parameters_to_context failed: %d", ret);
		avcodec_free_context(&m_codec_ctx);
		avformat_close_input(&m_fmt_ctx);
		m_fmt_ctx = nullptr;
		return -32;
	}

	// 12. 打开解码器
	av_dict_set(&m_codec_opts, "refcounted_frames", "1", 0);
	ret = avcodec_open2(m_codec_ctx, codec, &m_codec_opts);
	if (ret < 0) {
		hloge("avcodec_open2 failed: %d", ret);
		avcodec_free_context(&m_codec_ctx);
		avformat_close_input(&m_fmt_ctx);
		m_fmt_ctx = nullptr;
		return -33;
	}

	// 13. 创建格式转换上下文
	m_sws_ctx = sws_getContext(m_width, m_height, m_src_pix_fmt,
		m_dst_width, m_dst_height, m_dst_pix_fmt,
		SWS_BICUBIC, nullptr, nullptr, nullptr);
	if (!m_sws_ctx) {
		hloge("sws_getContext failed");
		avcodec_free_context(&m_codec_ctx);
		avformat_close_input(&m_fmt_ctx);
		m_fmt_ctx = nullptr;
		return -40;
	}

	// 14. 获取媒体信息
	if (video_stream->avg_frame_rate.num && video_stream->avg_frame_rate.den) {
		m_fps = video_stream->avg_frame_rate.num / video_stream->avg_frame_rate.den;
	}

	if (video_stream->duration > 0) {
		m_duration = (int64_t)(video_stream->duration * av_q2d(m_video_time_base) * 1000);
	}

	return 0;
}

// 关闭输入源并释放资源
void FFmpegDecoder::close_input() {
	// 释放格式转换上下文
	if (m_sws_ctx) {
		sws_freeContext(m_sws_ctx);
		m_sws_ctx = nullptr;
	}

	// 释放解码器
	if (m_codec_ctx) {
		avcodec_close(m_codec_ctx);
		avcodec_free_context(&m_codec_ctx);
		m_codec_ctx = nullptr;
	}

	// 释放格式上下文
	if (m_fmt_ctx) {
		avformat_close_input(&m_fmt_ctx);
		avformat_free_context(m_fmt_ctx);
		m_fmt_ctx = nullptr;
	}

	// 释放数据包和帧
	if (m_packet) {
		av_packet_free(&m_packet);
		m_packet = nullptr;
	}

	if (m_frame) {
		av_frame_free(&m_frame);
		m_frame = nullptr;
	}

	// 释放字典参数
	if (m_fmt_opts) {
		av_dict_free(&m_fmt_opts);
		m_fmt_opts = nullptr;
	}

	if (m_codec_opts) {
		av_dict_free(&m_codec_opts);
		m_codec_opts = nullptr;
	}

	// 清空帧队列
	clear_frames();
}

// 开始解码线程
int FFmpegDecoder::start() {
	if (m_running) {
		hloge("Decoder is already running");
		return -1;
	}

	if (!m_fmt_ctx || !m_codec_ctx) {
		hloge("Player not initialized");
		return -2;
	}

	m_quit = false;
	m_eof = false;
	m_running = true;

	// 启动解码线程
	m_decode_thread = std::thread(&FFmpegDecoder::decode_loop, this);

	hlogi("Decoder started");
	return 0;
}

// 停止解码
void FFmpegDecoder::stop() {
	if (!m_running) return;

	hlogi("Stopping decoder...");

	// 通知线程退出
	m_quit = true;
	m_running = false;

	// 唤醒等待的线程
	m_queue_cv.notify_all();

	// 等待线程结束
	if (m_decode_thread.joinable()) {
		m_decode_thread.join();
	}

	// 清理资源
	close_input();

	hlogi("Decoder stopped");
}

// 解码线程主循环
void FFmpegDecoder::decode_loop() {
	hlogi("Decode thread started");

	while (!m_quit && !m_eof && m_error_code == 0) {
		//av_init_packet(m_packet);
		//m_packet = { 0 };
		// 设置中断回调
		m_fmt_ctx->interrupt_callback.callback = interrupt_callback;
		m_fmt_ctx->interrupt_callback.opaque = this;
		m_block_starttime = time(nullptr);

		// 读取数据包
		int ret = av_read_frame(m_fmt_ctx, m_packet);
		m_fmt_ctx->interrupt_callback.callback = nullptr;

		if (ret < 0) {
			if (ret == AVERROR_EOF || avio_feof(m_fmt_ctx->pb)) {
				hlogi("End of file");
				m_eof = true;
			}
			else {
				hloge("av_read_frame failed: %d", ret);
				m_error_code = ret;
			}
			break;
		}

		// 只处理视频流
		if (m_packet->stream_index != m_video_stream_index) {
			av_packet_unref(m_packet);
			continue;
		}

		// 发送数据包到解码器
		ret = avcodec_send_packet(m_codec_ctx, m_packet);
		av_packet_unref(m_packet); // 释放数据包

		if (ret < 0) {
			hloge("avcodec_send_packet failed: %d", ret);
			m_error_code = ret;
			break;
		}

		// 接收解码后的帧
		ret = avcodec_receive_frame(m_codec_ctx, m_frame);
		if (ret == AVERROR(EAGAIN)) {
			continue; // 需要更多数据
		}
		else if (ret < 0) {
			hloge("avcodec_receive_frame failed: %d", ret);
			m_error_code = ret;
			break;
		}

		// 转换帧格式并加入队列
		DecodedFrame decoded_frame;
		if (convert_frame(m_frame, decoded_frame)) {
			std::unique_lock<std::mutex> lock(m_queue_mutex);

			// 限制队列大小，避免内存暴涨
			while (m_frame_queue.size() >= m_config.frame_queue_size && !m_quit) {
				m_queue_cv.wait_for(lock, std::chrono::milliseconds(10));
			}

			if (!m_quit) {
				m_frame_queue.push(std::move(decoded_frame));
				m_queue_cv.notify_one();
			}
		}

		av_frame_unref(m_frame);
	}

	// 通知所有等待的线程
	m_queue_cv.notify_all();
	hlogi("Decode thread exited");
}

// 转换AVFrame到DecodedFrame
bool FFmpegDecoder::convert_frame(AVFrame* av_frame, DecodedFrame& out_frame) {
	if (!av_frame || !m_sws_ctx) {
		return false;
	}

	// 设置基本信息
	out_frame.width = m_dst_width;
	out_frame.height = m_dst_height;
	out_frame.timestamp = (int64_t)(av_frame->pts * av_q2d(m_video_time_base) * 1000);

	// 根据目标格式分配内存
	if (m_dst_pix_fmt == AV_PIX_FMT_YUV420P) {
		out_frame.format = PIX_FMT_YUV420P;
		int y_size = m_dst_width * m_dst_height;
		out_frame.data_size = y_size * 3 / 2;
		out_frame.data = new uint8_t[out_frame.data_size];

		// 设置YUV数据指针
		uint8_t* data[3] = {
			out_frame.data,
			out_frame.data + y_size,
			out_frame.data + y_size + y_size / 4
		};
		int linesize[3] = { m_dst_width, m_dst_width / 2, m_dst_width / 2 };

		// 格式转换
		int ret = sws_scale(m_sws_ctx, av_frame->data, av_frame->linesize,
			0, av_frame->height, data, linesize);
		if (ret <= 0) {
			delete[] out_frame.data;
			out_frame.data = nullptr;
			return false;
		}

		// 保存行大小
		out_frame.linesize[0] = linesize[0];
		out_frame.linesize[1] = linesize[1];
		out_frame.linesize[2] = linesize[2];
	}
	else if (m_dst_pix_fmt == AV_PIX_FMT_BGR24) {
		out_frame.format = PIX_FMT_BGR24;
		out_frame.data_size = m_dst_width * m_dst_height * 3;
		out_frame.data = new uint8_t[out_frame.data_size];

		uint8_t* data[1] = { out_frame.data };
		int linesize[1] = { m_dst_width * 3 };

		int ret = sws_scale(m_sws_ctx, av_frame->data, av_frame->linesize,
			0, av_frame->height, data, linesize);
		if (ret <= 0) {
			delete[] out_frame.data;
			out_frame.data = nullptr;
			return false;
		}

		out_frame.linesize[0] = linesize[0];
	}
	else {
		return false;
	}

	return true;
}

// 获取解码后的帧
bool FFmpegDecoder::get_frame(DecodedFrame& frame) {
	std::unique_lock<std::mutex> lock(m_queue_mutex);

	// 等待有帧或停止
	while (m_frame_queue.empty() && m_running && !m_eof && m_error_code == 0) {
		m_queue_cv.wait(lock);
	}

	// 检查是否有帧
	if (!m_frame_queue.empty()) {
		frame = std::move(m_frame_queue.front());
		m_frame_queue.pop();
		return true;
	}

	return false;
}

// 清空帧队列
void FFmpegDecoder::clear_frames() {
	std::lock_guard<std::mutex> lock(m_queue_mutex);
	std::queue<DecodedFrame> empty_queue;
	std::swap(m_frame_queue, empty_queue);
}

// 跳转播放位置
int FFmpegDecoder::seek(int64_t ms) {
	if (!m_fmt_ctx || m_video_stream_index < 0) {
		return -1;
	}

	if (ms < 0) ms = 0;
	if (ms > m_duration) ms = m_duration;

	// 计算目标时间戳（转换为FFmpeg时间基）
	int64_t target_ts = (int64_t)(ms / 1000.0 / av_q2d(m_video_time_base));

	// 清空帧队列
	clear_frames();

	// 执行seek
	int ret = av_seek_frame(m_fmt_ctx, m_video_stream_index, target_ts, AVSEEK_FLAG_BACKWARD);
	if (ret < 0) {
		hloge("av_seek_frame failed: %d", ret);
		m_error_code = ret;
		return ret;
	}

	// 刷新解码器
	if (m_codec_ctx) {
		avcodec_flush_buffers(m_codec_ctx);
	}

	hlogi("Seek to %lldms (ts: %lld)", ms, target_ts);
	return 0;
}



// ====================== C 接口导出部分 ======================
extern "C" {

	// 创建解码器实例
	OPENCVFFMPEGTOOLS_API void* FFmpegDecoder_Create()
	{
		return new FFmpegDecoder();
	}

	// 销毁解码器实例
	OPENCVFFMPEGTOOLS_API void FFmpegDecoder_Destroy(void* decoder)
	{
		if (decoder) {
			delete static_cast<FFmpegDecoder*>(decoder);
		}
	}

	// 初始化解码器
	// 参数：decoder-解码器实例，config-配置结构体指针
	// 返回值：0成功，非0失败（错误码同原有init函数）
	OPENCVFFMPEGTOOLS_API int FFmpegDecoder_Init(void* decoder, const PlayerConfig* config)
	{
		if (!decoder || !config) {
			return -1;
		}
		return static_cast<FFmpegDecoder*>(decoder)->init(*config);
	}

	// 启动解码线程
	// 参数：decoder-解码器实例
	// 返回值：0成功，非0失败
	OPENCVFFMPEGTOOLS_API int FFmpegDecoder_Start(void* decoder)
	{
		if (!decoder) {
			return -1;
		}
		return static_cast<FFmpegDecoder*>(decoder)->start();
	}

	// 停止解码
	// 参数：decoder-解码器实例
	OPENCVFFMPEGTOOLS_API void FFmpegDecoder_Stop(void* decoder)
	{
		if (decoder) {
			static_cast<FFmpegDecoder*>(decoder)->stop();
		}
	}

	// 获取解码后的帧
	// 参数：decoder-解码器实例，frame-输出帧结构体指针
	// 返回值：true(1)成功获取帧，false(0)无帧/停止/出错
	OPENCVFFMPEGTOOLS_API int FFmpegDecoder_GetFrame(void* decoder, DecodedFrame* frame)
	{
		if (!decoder || !frame) {
			return 0;
		}
		return static_cast<FFmpegDecoder*>(decoder)->get_frame(*frame) ? 1 : 0;
	}

	// 跳转播放位置
	// 参数：decoder-解码器实例，ms-目标时间（毫秒）
	// 返回值：0成功，非0失败
	OPENCVFFMPEGTOOLS_API int FFmpegDecoder_Seek(void* decoder, int64_t ms)
	{
		if (!decoder) {
			return -1;
		}
		return static_cast<FFmpegDecoder*>(decoder)->seek(ms);
	}

	// 获取媒体总时长（毫秒）
	// 参数：decoder-解码器实例
	// 返回值：时长（毫秒），失败返回-1
	OPENCVFFMPEGTOOLS_API int64_t FFmpegDecoder_GetDuration(void* decoder)
	{
		if (!decoder) {
			return -1;
		}
		return static_cast<FFmpegDecoder*>(decoder)->get_duration();
	}

	// 获取视频宽度
	// 参数：decoder-解码器实例
	// 返回值：宽度，失败返回-1
	OPENCVFFMPEGTOOLS_API int FFmpegDecoder_GetWidth(void* decoder)
	{
		if (!decoder) {
			return -1;
		}
		return static_cast<FFmpegDecoder*>(decoder)->get_width();
	}

	// 获取视频高度
	// 参数：decoder-解码器实例
	// 返回值：高度，失败返回-1
	OPENCVFFMPEGTOOLS_API int FFmpegDecoder_GetHeight(void* decoder)
	{
		if (!decoder) {
			return -1;
		}
		return static_cast<FFmpegDecoder*>(decoder)->get_height();
	}

	// 获取帧率
	// 参数：decoder-解码器实例
	// 返回值：帧率，失败返回-1
	OPENCVFFMPEGTOOLS_API int FFmpegDecoder_GetFps(void* decoder)
	{
		if (!decoder) {
			return -1;
		}
		return static_cast<FFmpegDecoder*>(decoder)->get_fps();
	}

	// 获取错误码
	// 参数：decoder-解码器实例
	// 返回值：错误码，0表示无错误
	OPENCVFFMPEGTOOLS_API int FFmpegDecoder_GetErrorCode(void* decoder)
	{
		if (!decoder) {
			return -1;
		}
		return static_cast<FFmpegDecoder*>(decoder)->get_error_code();
	}

	// 清空帧队列
	// 参数：decoder-解码器实例
	OPENCVFFMPEGTOOLS_API void FFmpegDecoder_ClearFrames(void* decoder)
	{
		if (decoder) {
			static_cast<FFmpegDecoder*>(decoder)->clear_frames();
		}
	}

	// 释放解码帧的内存（C端调用后需释放帧数据）
	// 参数：frame-解码帧结构体指针
	OPENCVFFMPEGTOOLS_API void FFmpegDecoder_FreeFrameData(DecodedFrame* frame)
	{
		if (frame && frame->data) {
			delete[] frame->data;
			frame->data = nullptr;
			frame->data_size = 0;
		}
	}

} // extern "C"

//example
/*
#include <iostream>

int main() {
	// 1. 创建播放器配置
	PlayerConfig config;
	config.src = "2.mp4";                  // 本地文件/RTSP地址/设备名
	config.type = MEDIA_TYPE_FILE;            // 媒体类型
	config.decode_mode = SOFTWARE_DECODE;     // 软件解码
	config.dst_format = PIX_FMT_YUV420P;      // 输出YUV420P格式
	config.block_timeout = 10;                // 阻塞超时10秒
	config.frame_queue_size = 5;              // 帧队列大小

	// 2. 创建解码器实例
	FFmpegDecoder decoder;

	// 3. 初始化
	int ret = decoder.init(config);
	if (ret != 0) {
		std::cerr << "Init decoder failed: " << ret << std::endl;
		return -1;
	}

	// 4. 开始解码
	ret = decoder.start();
	if (ret != 0) {
		std::cerr << "Start decoder failed: " << ret << std::endl;
		return -1;
	}

	// 5. 循环获取解码后的帧
	int frame_count = 0;
	while (decoder.is_running() && !decoder.is_eof() && decoder.get_error_code() == 0) {
		DecodedFrame frame;
		if (decoder.get_frame(frame)) {
			frame_count++;
			std::cout << "Get frame " << frame_count
				<< ": " << frame.width << "x" << frame.height
				<< ", ts=" << frame.timestamp << "ms"
				<< ", size=" << frame.data_size << " bytes" << std::endl;

			// TODO: 处理帧数据（保存文件/网络传输/其他处理）
			// frame.data 就是解码后的像素数据
		}
	}

	// 6. 停止解码器
	decoder.stop();

	std::cout << "Decode finished, total frames: " << frame_count << std::endl;
	if (decoder.get_error_code() != 0) {
		std::cerr << "Decode error: " << decoder.get_error_code() << std::endl;
	}

	return 0;
}



*/
