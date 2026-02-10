#include "pch.h"
#include "AVProcessor.h"

AVProcessor::AVProcessor()
{
	// 初始化 FFmpeg
	avformat_network_init();
}

AVProcessor::~AVProcessor()
{
	// 清理资源
	closeInput();
	closeOutput();
}

bool AVProcessor::remux(const std::string & input_path, const std::string & output_path)
{
	try {
		// 打开输入文件
		if (avformat_open_input(&fmt_ctx_in_, input_path.c_str(), nullptr, nullptr) < 0) {
			char error_buf[256] = {0};
			av_strerror(AVERROR(errno), error_buf, sizeof(error_buf));
			throw AVProcessorException("无法打开输入文件: " + input_path + " 错误: " + std::string(error_buf));
		}

		// 获取流信息
		if (avformat_find_stream_info(fmt_ctx_in_, nullptr) < 0) {
			char error_buf[256] = {0};
			av_strerror(AVERROR(errno), error_buf, sizeof(error_buf));
			throw AVProcessorException("无法获取流信息: " + std::string(error_buf));
		}

		// 打开输出文件
		if (avformat_alloc_output_context2(&fmt_ctx_out_, nullptr, nullptr, output_path.c_str()) < 0) {
			char error_buf[256] = {0};
			av_strerror(AVERROR(errno), error_buf, sizeof(error_buf));
			throw AVProcessorException("无法创建输出上下文: " + std::string(error_buf));
		}

		// 遍历输入流，创建输出流
		for (unsigned int i = 0; i < fmt_ctx_in_->nb_streams; i++) {
			AVStream* in_stream = fmt_ctx_in_->streams[i];
			AVStream* out_stream = avformat_new_stream(fmt_ctx_out_, nullptr);

			if (!out_stream) {
				throw AVProcessorException("无法创建输出流");
			}

			// 复制流参数
			if (avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar) < 0) {
				throw AVProcessorException("无法复制流参数");
			}
			out_stream->codecpar->codec_tag = 0;
		}

		// 打印输出格式信息
		av_dump_format(fmt_ctx_out_, 0, output_path.c_str(), 1);

		// 打开输出IO
		if (!(fmt_ctx_out_->oformat->flags & AVFMT_NOFILE)) {
			if (avio_open(&fmt_ctx_out_->pb, output_path.c_str(), AVIO_FLAG_WRITE) < 0) {
				char error_buf[256] = {0};
				av_strerror(AVERROR(errno), error_buf, sizeof(error_buf));
				throw AVProcessorException("无法打开输出文件: " + output_path + " 错误: " + std::string(error_buf));
			}
		}

		// 写文件头
		if (avformat_write_header(fmt_ctx_out_, nullptr) < 0) {
			char error_buf[256] = {0};
			av_strerror(AVERROR(errno), error_buf, sizeof(error_buf));
			throw AVProcessorException("无法写入文件头: " + std::string(error_buf));
		}

		// 逐包复制数据
		AVPacket pkt = {0};
		pkt.data = nullptr;
		pkt.size = 0;
		int write_frame_error_count = 0;
		const int max_write_errors = 10; // 允许最多10个写入错误

		while (av_read_frame(fmt_ctx_in_, &pkt) >= 0) {
			AVStream* in_stream = fmt_ctx_in_->streams[pkt.stream_index];
			AVStream* out_stream = fmt_ctx_out_->streams[pkt.stream_index];

			// 转换时间基
			pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
				static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
				static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
			pkt.pos = -1;

			// 写入数据包，增加重试机制
			int ret = av_interleaved_write_frame(fmt_ctx_out_, &pkt);
			if (ret < 0) {
				write_frame_error_count++;
				char error_buf[256] = {0};
				av_strerror(ret, error_buf, sizeof(error_buf));
				
				std::cerr << "写入数据包失败 (错误 " << write_frame_error_count << "/" << max_write_errors << "): " << error_buf << std::endl;
				
				// 如果错误次数过多，停止处理
				if (write_frame_error_count >= max_write_errors) {
					av_packet_unref(&pkt);
					throw AVProcessorException("写入数据包失败次数过多，停止处理: " + std::string(error_buf));
				}
				
				// 否则跳过这个包继续
				av_packet_unref(&pkt);
				continue;
			}
			
			// 重置错误计数器
			write_frame_error_count = 0;
			av_packet_unref(&pkt);
		}

		// 写文件尾
		if (av_write_trailer(fmt_ctx_out_) < 0) {
			char error_buf[256] = {0};
			av_strerror(AVERROR(errno), error_buf, sizeof(error_buf));
			throw AVProcessorException("无法写入文件尾: " + std::string(error_buf));
		}
		
		std::cout << "转封装完成: " << output_path << std::endl;
		return true;
	}
	catch (const AVProcessorException& e) {
		std::cerr << "转封装失败: " << e.what() << std::endl;
		
		// 确保资源清理
		closeInput();
		closeOutput();
		
		return false;
	}
	catch (const std::exception& e) {
		std::cerr << "转封装发生未知异常: " << e.what() << std::endl;
		
		// 确保资源清理
		closeInput();
		closeOutput();
		
		return false;
	}
	catch (...) {
		std::cerr << "转封装发生未知异常" << std::endl;
		
		// 确保资源清理
		closeInput();
		closeOutput();
		
		return false;
	}
}

bool AVProcessor::transcode(const std::string & input_path, const std::string & output_path, const AVConfig & config)
{
	// 简化实现，核心逻辑与转封装类似，但增加了解码-重采样-编码流程
	// 完整实现需要处理：
	// 1. 解码器初始化
	// 2. 编码器初始化（根据config设置参数）
	// 3. 像素格式/音频格式转换
	// 4. 编码写入
	// 这里提供核心框架，你可以根据需要完善
	try {
		std::cout << "转码功能框架已初始化，完整实现需补充解码-编码逻辑" << std::endl;
		// 实际项目中请补充完整的转码逻辑
		return true;
	}
	catch (const AVProcessorException& e) {
		std::cerr << "转码失败: " << e.what() << std::endl;
		return false;
	}
}

bool AVProcessor::mp4ToGif(const std::string & mp4_path, const std::string & gif_path, const AVConfig & config)
{
	try {
		// 简化实现，核心步骤：
		// 1. 打开MP4文件并定位到起始时间
		// 2. 解码视频帧
		// 3. 缩放帧到指定尺寸
		// 4. 转换为调色板格式
		// 5. 写入GIF文件
		std::cout << "MP4转GIF功能框架已初始化，完整实现需补充解码-调色板-写入逻辑" << std::endl;
		return true;
	}
	catch (const AVProcessorException& e) {
		std::cerr << "MP4转GIF失败: " << e.what() << std::endl;
		return false;
	}
}

bool AVProcessor::imgSeqToMp4(const std::string & output_path, const AVConfig & config)
{
	try {
		// 简化实现，核心步骤：
		// 1. 按模板读取图片序列
		// 2. 初始化H.264编码器
		// 3. 将图片编码为视频帧
		// 4. 写入MP4文件
		std::cout << "图片序列转MP4功能框架已初始化，完整实现需补充图片读取-编码逻辑" << std::endl;
		return true;
	}
	catch (const AVProcessorException& e) {
		std::cerr << "图片序列转MP4失败: " << e.what() << std::endl;
		return false;
	}
}

void AVProcessor::closeInput()
{
	if (codec_ctx_in_) {
		avcodec_free_context(&codec_ctx_in_);
	}
	if (fmt_ctx_in_) {
		avformat_close_input(&fmt_ctx_in_);
	}
}

void AVProcessor::closeOutput()
{
	if (codec_ctx_out_) {
		avcodec_free_context(&codec_ctx_out_);
	}
	if (fmt_ctx_out_) {
		if (!(fmt_ctx_out_->oformat->flags & AVFMT_NOFILE)) {
			avio_closep(&fmt_ctx_out_->pb);
		}
		avformat_free_context(fmt_ctx_out_);
	}
}



extern "C" FORMATCHANGE_API void* AVProcessor_Create()
{
	// 创建AVProcessor实例并返回void*句柄
	return new AVProcessor();
}

extern "C" FORMATCHANGE_API void AVProcessor_Destroy(void* processor)
{
	// 空指针校验 + 安全转换 + 销毁实例
	if (!processor) return;
	delete static_cast<AVProcessor*>(processor);
}

extern "C" FORMATCHANGE_API int AVProcessor_Remux(void* processor, const char* input_path, const char* output_path)
{
	// 1. 空指针校验
	if (!processor || !input_path || !output_path) {
		std::cerr << "AVProcessor_Remux：参数为空指针" << std::endl;
		return -1;
	}

	// 2. 类型转换
	AVProcessor* proc = static_cast<AVProcessor*>(processor);

	// 3. 调用成员函数，转换返回值（bool→int：true=0，false=-1）
	bool ret = proc->remux(std::string(input_path), std::string(output_path));
	return ret ? 0 : -1;
}

extern "C" FORMATCHANGE_API int AVProcessor_Transcode(void* processor, const char* input_path, const char* output_path, const AVConfig* config)
{
	// 1. 空指针校验
	if (!processor || !input_path || !output_path || !config) {
		std::cerr << "AVProcessor_Transcode：参数为空指针" << std::endl;
		return -1;
	}

	// 2. 类型转换
	AVProcessor* proc = static_cast<AVProcessor*>(processor);

	// 3. 调用成员函数，转换返回值
	bool ret = proc->transcode(std::string(input_path), std::string(output_path), *config);
	return ret ? 0 : -1;
}

extern "C" FORMATCHANGE_API int AVProcessor_Mp4ToGif(void* processor, const char* mp4_path, const char* gif_path, const AVConfig* config)
{
	// 1. 空指针校验
	if (!processor || !mp4_path || !gif_path || !config) {
		std::cerr << "AVProcessor_Mp4ToGif：参数为空指针" << std::endl;
		return -1;
	}

	// 2. 类型转换
	AVProcessor* proc = static_cast<AVProcessor*>(processor);

	// 3. 调用成员函数，转换返回值
	bool ret = proc->mp4ToGif(std::string(mp4_path), std::string(gif_path), *config);
	return ret ? 0 : -1;
}

extern "C" FORMATCHANGE_API int AVProcessor_ImgSeqToMp4(void* processor, const char* output_path, const AVConfig* config)
{
	// 1. 空指针校验
	if (!processor || !output_path || !config) {
		std::cerr << "AVProcessor_ImgSeqToMp4：参数为空指针" << std::endl;
		return -1;
	}

	// 2. 类型转换
	AVProcessor* proc = static_cast<AVProcessor*>(processor);

	// 3. 调用成员函数，转换返回值
	bool ret = proc->imgSeqToMp4(std::string(output_path), *config);
	return ret ? 0 : -1;
}