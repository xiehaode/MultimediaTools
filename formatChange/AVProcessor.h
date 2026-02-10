#pragma once
#include "pch.h"
#include "formatChange.h"
class AVProcessorException : public std::exception {
public:
	explicit AVProcessorException(const std::string& msg) : msg_(msg) {}
	const char* what() const noexcept override {
		return msg_.c_str();
	}
private:
	std::string msg_;
};

class AVProcessor {
public:
	AVProcessor();

	~AVProcessor();

	AVProcessor(const AVProcessor&) = delete;
	AVProcessor& operator=(const AVProcessor&) = delete;

	//AVProcessor(AVProcessor&&) noexcept;
	//AVProcessor& operator=(AVProcessor&&) noexcept;

	/**
	 * @brief 音视频转封装（仅改变容器格式，不重新编码）
	 * @param input_path 输入文件路径
	 * @param output_path 输出文件路径
	 * @return 成功返回true，失败返回false
	 */
	bool remux(const std::string& input_path, const std::string& output_path);

	/**
	 * @brief 音视频转码（重新编码，可修改编码格式/参数）
	 * @param input_path 输入文件路径
	 * @param output_path 输出文件路径
	 * @param config 转码配置参数
	 * @return 成功返回true，失败返回false
	 */
	bool transcode(const std::string& input_path, const std::string& output_path, const AVConfig& config);

	/**
	 * @brief MP4转GIF
	 * @param mp4_path MP4文件路径
	 * @param gif_path GIF输出路径
	 * @param config 转换配置参数（含尺寸、帧率、延迟等）
	 * @return 成功返回true，失败返回false
	 */
	bool mp4ToGif(const std::string& mp4_path, const std::string& gif_path, const AVConfig& config);

	/**
	 * @brief 图片序列转MP4
	 * @param output_path MP4输出路径
	 * @param config 转换配置参数
	 * @return 成功返回true，失败返回false
	 */
	bool imgSeqToMp4(const std::string& output_path, const AVConfig& config);

private:
	void closeInput();

	void closeOutput();

	// FFmpeg核心上下文对象
	AVFormatContext* fmt_ctx_in_ = nullptr;   // 输入格式上下文
	AVFormatContext* fmt_ctx_out_ = nullptr;  // 输出格式上下文
	AVCodecContext* codec_ctx_in_ = nullptr;  // 输入编解码器上下文
	AVCodecContext* codec_ctx_out_ = nullptr; // 输出编解码器上下文
};


