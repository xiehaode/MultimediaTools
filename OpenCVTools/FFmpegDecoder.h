#pragma once

#include "pch.h"
// 日志宏定义（简化版，可替换为自定义日志）
#define hlogi(...) printf("[INFO] " __VA_ARGS__); printf("\n")
#define hloge(...) printf("[ERROR] " __VA_ARGS__); printf("\n")

// 解码模式枚举
enum DecodeMode {
	SOFTWARE_DECODE = 0,
	HARDWARE_DECODE_CUVID,
	HARDWARE_DECODE_QSV
};

// 像素格式枚举
enum PixelFormat {
	PIX_FMT_YUV420P = 0,
	PIX_FMT_BGR24
};

// 解码后的帧数据结构（对外暴露）
struct DecodedFrame {
	PixelFormat format;    // 像素格式
	int width;             // 帧宽度
	int height;            // 帧高度
	int64_t timestamp;     // 时间戳（毫秒）
	uint8_t* data;         // 帧数据缓冲区
	int data_size;         // 数据大小
	int linesize[3];       // 行大小（YUV使用）

	// 构造函数
	DecodedFrame() : format(PIX_FMT_YUV420P), width(0), height(0),
		timestamp(0), data(nullptr), data_size(0) {
		linesize[0] = linesize[1] = linesize[2] = 0;
	}

	// 析构函数
	~DecodedFrame() {
		if (data) {
			delete[] data;
			data = nullptr;
		}
	}

	// 禁止拷贝（避免浅拷贝导致内存问题）
	DecodedFrame(const DecodedFrame&) = delete;
	DecodedFrame& operator=(const DecodedFrame&) = delete;

	// 移动构造（支持队列传递）
	DecodedFrame(DecodedFrame&& other) noexcept {
		format = other.format;
		width = other.width;
		height = other.height;
		timestamp = other.timestamp;
		data = other.data;
		data_size = other.data_size;
		linesize[0] = other.linesize[0];
		linesize[1] = other.linesize[1];
		linesize[2] = other.linesize[2];

		other.data = nullptr;
		other.data_size = 0;
	}

	DecodedFrame& operator=(DecodedFrame&& other) noexcept {
		if (this != &other) {
			// 释放当前资源
			if (data) delete[] data;

			// 移动资源
			format = other.format;
			width = other.width;
			height = other.height;
			timestamp = other.timestamp;
			data = other.data;
			data_size = other.data_size;
			linesize[0] = other.linesize[0];
			linesize[1] = other.linesize[1];
			linesize[2] = other.linesize[2];

			other.data = nullptr;
			other.data_size = 0;
		}
		return *this;
	}
};

// 媒体源类型
enum MediaType {
	MEDIA_TYPE_FILE = 0,     // 本地文件
	MEDIA_TYPE_NETWORK,      // 网络流（RTSP/HTTP等）
	MEDIA_TYPE_CAPTURE       // 设备采集（摄像头）
};

// 播放器配置参数
struct PlayerConfig {
	std::string src;         // 源地址（文件路径/RTSP地址/设备名）
	MediaType type;          // 媒体类型
	DecodeMode decode_mode;  // 解码模式
	PixelFormat dst_format;  // 目标像素格式（YUV420P/BGR24）
	int block_timeout = 10;  // 阻塞超时时间（秒）
	int frame_queue_size = 5; // 帧队列最大缓存数
};

// 纯解码播放器类（无显示逻辑）
class FFmpegDecoder {
public:
	FFmpegDecoder();
	~FFmpegDecoder();

	// 禁止拷贝
	FFmpegDecoder(const FFmpegDecoder&) = delete;
	FFmpegDecoder& operator=(const FFmpegDecoder&) = delete;

	/**
	 * @brief 初始化播放器
	 * @param config 配置参数
	 * @return 0成功，非0失败
	 */
	int init(const PlayerConfig& config);

	/**
	 * @brief 开始解码（启动解码线程）
	 * @return 0成功，非0失败
	 */
	int start();

	/**
	 * @brief 停止解码（停止线程并清理）
	 */
	void stop();

	/**
	 * @brief 获取解码后的帧（阻塞直到有帧或停止）
	 * @param frame 输出帧数据（移动语义）
	 * @return true成功获取，false已停止/无帧
	 */
	bool get_frame(DecodedFrame& frame);

	/**
	 * @brief 清空帧队列
	 */
	void clear_frames();

	/**
	 * @brief 跳转播放位置
	 * @param ms 目标时间戳（毫秒）
	 * @return 0成功，非0失败
	 */
	int seek(int64_t ms);

	/**
	 * @brief 获取播放器状态
	 */
	bool is_running() const { return m_running; }
	bool is_eof() const { return m_eof; }
	int get_error_code() const { return m_error_code; }

	/**
	 * @brief 获取媒体信息
	 */
	int get_width() const { return m_width; }
	int get_height() const { return m_height; }
	int get_fps() const { return m_fps; }
	int64_t get_duration() const { return m_duration; } // 总时长（毫秒）

private:
	// FFmpeg核心上下文
	AVFormatContext* m_fmt_ctx = nullptr;
	AVCodecContext* m_codec_ctx = nullptr;
	SwsContext* m_sws_ctx = nullptr;
	AVPacket* m_packet = nullptr;
	AVFrame* m_frame = nullptr;
	AVDictionary* m_fmt_opts = nullptr;
	AVDictionary* m_codec_opts = nullptr;

	// 配置参数
	PlayerConfig m_config;
	int m_video_stream_index = -1;
	int m_width = 0;
	int m_height = 0;
	int m_fps = 0;
	int64_t m_duration = 0; // 总时长（毫秒）
	AVRational m_video_time_base = { 0, 1 };
	AVPixelFormat m_src_pix_fmt = AV_PIX_FMT_NONE;
	AVPixelFormat m_dst_pix_fmt = AV_PIX_FMT_YUV420P;
	int m_dst_width = 0;
	int m_dst_height = 0;

	// 线程相关
	std::thread m_decode_thread;
	std::atomic<bool> m_running{ false };
	std::atomic<bool> m_quit{ false };
	std::atomic<bool> m_eof{ false };
	std::atomic<int> m_error_code{ 0 };
	std::atomic<time_t> m_block_starttime{ 0 };

	// 帧队列（线程安全）
	std::queue<DecodedFrame> m_frame_queue;
	std::mutex m_queue_mutex;
	std::condition_variable m_queue_cv;

	// FFmpeg全局初始化标志
	static std::atomic_flag s_ffmpeg_init;

	/**
	 * @brief 中断回调（防止FFmpeg阻塞API卡死）
	 */
	static int interrupt_callback(void* opaque);

	/**
	 * @brief 打开输入源并初始化解码器
	 */
	int open_input();

	/**
	 * @brief 关闭输入源并释放资源
	 */
	void close_input();

	/**
	 * @brief 解码线程主函数
	 */
	void decode_loop();

	/**
	 * @brief 将AVFrame转换为DecodedFrame
	 */
	bool convert_frame(AVFrame* av_frame, DecodedFrame& out_frame);
};

