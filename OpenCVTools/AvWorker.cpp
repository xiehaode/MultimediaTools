#include "pch.h"
#include "AvWorker.h"

#include <algorithm>
#include <vector>


AvWorker::AvWorker()
{

}

void AvWorker::initAv()
{

}

void AvWorker::finishAv()
{

}

// 调整视频分辨率函数 - 修复时间戳导致的时长异常问题
bool AvWorker::resize_video(const std::string& input_path, const std::string& output_path,
	int dst_width, int dst_height) {

	auto cleanup = [](AVFormatContext* in_fmt_ctx, AVFormatContext* out_fmt_ctx,
		AVCodecContext* in_codec_ctx, AVCodecContext* out_codec_ctx,
		SwsContext* sws_ctx, AVFrame* src_frame, AVFrame* dst_frame) {
		if (src_frame) av_frame_free(&src_frame);
		if (dst_frame) av_frame_free(&dst_frame);
		if (sws_ctx) sws_freeContext(sws_ctx);
		if (in_codec_ctx) avcodec_free_context(&in_codec_ctx);
		if (out_codec_ctx) avcodec_free_context(&out_codec_ctx);
		if (in_fmt_ctx) avformat_close_input(&in_fmt_ctx);
		if (out_fmt_ctx) {
			if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE))
				avio_closep(&out_fmt_ctx->pb);
			avformat_free_context(out_fmt_ctx);
		}
	};
	//  参数合法性校验 - 前置检查，失败直接返回
	if (input_path.empty() || output_path.empty()) {
		std::cerr << "错误：输入或输出文件路径不能为空" << std::endl;
		return false;
	}
	if (dst_width <= 0 || dst_height <= 0 ||
		dst_width % 2 != 0 || dst_height % 2 != 0) {
		std::cerr << "错误：目标分辨率必须是正偶数（H264要求）" << std::endl;
		return false;
	}

	//  初始化变量 - 所有指针初始化为nullptr
	avformat_network_init();
	AVFormatContext *in_fmt_ctx = nullptr, *out_fmt_ctx = nullptr;
	AVCodecContext *in_codec_ctx = nullptr, *out_codec_ctx = nullptr;
	SwsContext* sws_ctx = nullptr;
	AVFrame *src_frame = nullptr, *dst_frame = nullptr;
	AVPacket pkt = { 0 };
	pkt.data = nullptr;
	pkt.size = 0;

	int ret, video_stream_idx = -1;
	// 新增：帧计数，用于生成备用时间戳
	int64_t frame_index = 0;
	AVRational in_time_base;  // 输入流时间基
	AVRational out_time_base; // 输出流时间基

	//  打开输入文件
	ret = avformat_open_input(&in_fmt_ctx, input_path.c_str(), nullptr, nullptr);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "打开输入文件失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	//  获取流信息
	ret = avformat_find_stream_info(in_fmt_ctx, nullptr);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "获取流信息失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	// 查找视频流索引
	for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
		if (in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_idx = i;
			in_time_base = in_fmt_ctx->streams[video_stream_idx]->time_base; // 保存输入时间基
			break;
		}
	}
	if (video_stream_idx == -1) {
		std::cerr << "未找到视频流" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	//  初始化输入解码器
	AVCodecParameters* in_codecpar = in_fmt_ctx->streams[video_stream_idx]->codecpar;
	const AVCodec* in_codec = avcodec_find_decoder(in_codecpar->codec_id);
	if (!in_codec) {
		std::cerr << "找不到解码器" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	in_codec_ctx = avcodec_alloc_context3(in_codec);
	if (!in_codec_ctx) {
		std::cerr << "分配解码器上下文失败" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	ret = avcodec_parameters_to_context(in_codec_ctx, in_codecpar);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "复制解码器参数失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	ret = avcodec_open2(in_codec_ctx, in_codec, nullptr);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "打开解码器失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	//  创建输出上下文
	ret = avformat_alloc_output_context2(&out_fmt_ctx, nullptr, nullptr, output_path.c_str());
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "创建输出上下文失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	//  初始化输出编码器
	const AVCodec* out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!out_codec) {
		std::cerr << "找不到H264编码器" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	AVStream* out_stream = avformat_new_stream(out_fmt_ctx, out_codec);
	if (!out_stream) {
		std::cerr << "创建输出流失败" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	// 输出流时间基直接复用输入流的时间基（避免帧率反推的精度问题）
	out_time_base = in_time_base;
	out_stream->time_base = out_time_base;

	out_codec_ctx = avcodec_alloc_context3(out_codec);
	if (!out_codec_ctx) {
		std::cerr << "分配编码器上下文失败" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	// 配置输出编码器参数
	out_codec_ctx->codec_id = out_codec->id;
	out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	out_codec_ctx->width = dst_width;
	out_codec_ctx->height = dst_height;
	out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	// 编码器时间基和输出流对齐（使用1/帧率，而非av_inv_q）
	AVRational frame_rate = in_fmt_ctx->streams[video_stream_idx]->r_frame_rate;
	if (frame_rate.num == 0 || frame_rate.den == 0) {

		frame_rate = { 30, 1 }; // 兜底：默认30fps
	}
	out_codec_ctx->time_base = av_inv_q(frame_rate);
	out_codec_ctx->framerate = frame_rate;
	out_codec_ctx->bit_rate = in_codec_ctx->bit_rate;
	out_codec_ctx->gop_size = 10;
	out_codec_ctx->max_b_frames = 1;

	// 设置H264编码器参数（增加兼容性）
	if (out_codec_ctx->codec_id == AV_CODEC_ID_H264) {
		av_opt_set(out_codec_ctx->priv_data, "preset", "medium", 0);
		av_opt_set(out_codec_ctx->priv_data, "tune", "zerolatency", 0);
	}

	if (out_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
		out_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	// 打开编码器
	ret = avcodec_open2(out_codec_ctx, out_codec, nullptr);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "打开编码器失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	// 复制编码器参数到输出流
	ret = avcodec_parameters_from_context(out_stream->codecpar, out_codec_ctx);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "复制编码器参数到输出流失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	// 打开输出文件
	if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&out_fmt_ctx->pb, output_path.c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			char err_buf[1024] = { 0 };
			av_strerror(ret, err_buf, sizeof(err_buf));
			std::cerr << "打开输出文件失败: " << err_buf << std::endl;
			cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
			avformat_network_deinit();
			return false;
		}
	}

	// 写入文件头
	ret = avformat_write_header(out_fmt_ctx, nullptr);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "写入文件头失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	// 初始化缩放上下文
	sws_ctx = sws_getContext(in_codec_ctx->width, in_codec_ctx->height, in_codec_ctx->pix_fmt,
		dst_width, dst_height, out_codec_ctx->pix_fmt,
		SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!sws_ctx) {
		std::cerr << "创建缩放上下文失败" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	// 初始化帧
	src_frame = av_frame_alloc();
	if (!src_frame) {
		std::cerr << "分配源帧失败" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	dst_frame = av_frame_alloc();
	if (!dst_frame) {
		std::cerr << "分配目标帧失败" << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	dst_frame->width = dst_width;
	dst_frame->height = dst_height;
	dst_frame->format = out_codec_ctx->pix_fmt;

	ret = av_frame_get_buffer(dst_frame, 32);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "分配目标帧缓冲区失败: " << err_buf << std::endl;
		cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
		avformat_network_deinit();
		return false;
	}

	// 处理视频帧（解码→缩放→编码→写入）
	bool process_success = true;
	while (av_read_frame(in_fmt_ctx, &pkt) >= 0) {
		if (pkt.stream_index != video_stream_idx) {
			av_packet_unref(&pkt);
			continue;
		}

		// 解码帧
		ret = avcodec_send_packet(in_codec_ctx, &pkt);
		if (ret < 0) {
			char err_buf[1024] = { 0 };
			av_strerror(ret, err_buf, sizeof(err_buf));
			std::cerr << "发送数据包到解码器失败: " << err_buf << std::endl;
			av_packet_unref(&pkt);
			process_success = false;
			break;
		}

		while (avcodec_receive_frame(in_codec_ctx, src_frame) >= 0) {
			// 缩放帧
			sws_scale(sws_ctx, src_frame->data, src_frame->linesize, 0,
				in_codec_ctx->height, dst_frame->data, dst_frame->linesize);

			// PTS时间戳处理（增加边界校验，避免无效值）
			int64_t pts = src_frame->pts;
			if (pts == AV_NOPTS_VALUE) {
				// 无有效PTS时，用帧计数生成（frame_index * 时间基）
				pts = frame_index * av_q2d(out_codec_ctx->time_base) * AV_TIME_BASE;
				AVRational av_time_base_q = { 1, AV_TIME_BASE }; // 显式定义结构体变量
				pts = av_rescale_q(pts, av_time_base_q, out_codec_ctx->time_base);
			}
			else {
				// 有有效PTS时，正确转换到编码器时间基
				pts = av_rescale_q(pts, in_time_base, out_codec_ctx->time_base);
			}
			dst_frame->pts = pts;
			dst_frame->pkt_dts = dst_frame->pts; // DTS和PTS对齐（避免B帧导致的时序问题）
			frame_index++;

			// 编码帧
			ret = avcodec_send_frame(out_codec_ctx, dst_frame);
			if (ret < 0) {
				char err_buf[1024] = { 0 };
				av_strerror(ret, err_buf, sizeof(err_buf));
				std::cerr << "发送帧到编码器失败: " << err_buf << std::endl;
				process_success = false;
				break;
			}

			while (avcodec_receive_packet(out_codec_ctx, &pkt) >= 0) {
				pkt.stream_index = out_stream->index;
				// 数据包PTS/DTS转换到输出流时间基
				av_packet_rescale_ts(&pkt, out_codec_ctx->time_base, out_stream->time_base);
				pkt.dts = pkt.pts; // 兜底：DTS和PTS一致
				pkt.duration = av_rescale_q(1, out_codec_ctx->time_base, out_stream->time_base); // 设置帧时长

				ret = av_interleaved_write_frame(out_fmt_ctx, &pkt);
				if (ret < 0) {
					char err_buf[1024] = { 0 };
					av_strerror(ret, err_buf, sizeof(err_buf));
					std::cerr << "写入帧失败: " << err_buf << std::endl;
					process_success = false;
					break;
				}
				av_packet_unref(&pkt);
			}
			if (!process_success) break;
		}
		av_packet_unref(&pkt);
		if (!process_success) break;
	}

	//  刷新编码器
	if (process_success) {
		avcodec_send_frame(out_codec_ctx, nullptr);
		while (avcodec_receive_packet(out_codec_ctx, &pkt) >= 0) {
			pkt.stream_index = out_stream->index;
			av_packet_rescale_ts(&pkt, out_codec_ctx->time_base, out_stream->time_base);
			pkt.dts = pkt.pts;
			pkt.duration = av_rescale_q(1, out_codec_ctx->time_base, out_stream->time_base);

			ret = av_interleaved_write_frame(out_fmt_ctx, &pkt);
			if (ret < 0) {
				char err_buf[1024] = { 0 };
				av_strerror(ret, err_buf, sizeof(err_buf));
				std::cerr << "刷新编码器写入帧失败: " << err_buf << std::endl;
				process_success = false;
				break;
			}
			av_packet_unref(&pkt);
		}
	}

	// 写入文件尾
	if (process_success) {
		ret = av_write_trailer(out_fmt_ctx);
		if (ret < 0) {
			char err_buf[1024] = { 0 };
			av_strerror(ret, err_buf, sizeof(err_buf));
			std::cerr << "写入文件尾失败: " << err_buf << std::endl;
			process_success = false;
		}
	}

	// 统一清理资源
	cleanup(in_fmt_ctx, out_fmt_ctx, in_codec_ctx, out_codec_ctx, sws_ctx, src_frame, dst_frame);
	avformat_network_deinit();

	return process_success;
}

// 保存RGB数据为BMP图片（修复头格式问题，兼容ImageMagick）
bool AvWorker::SaveFrameToBmp(const uint8_t* rgb_data, int width, int height, const std::string& output_path) {
	// 严格按照BMP标准定义结构体（禁用编译器对齐）
#pragma pack(push, 1) // 强制1字节对齐，符合BMP标准
// BMP文件头（固定14字节）
	typedef struct {
		uint16_t bfType;      // 必须为0x4D42 ("BM")
		uint32_t bfSize;      // 文件总大小
		uint16_t bfReserved1; // 保留，0
		uint16_t bfReserved2; // 保留，0
		uint32_t bfOffBits;   // 像素数据偏移量
	} BITMAPFILEHEADER;

	// BMP信息头（固定40字节）
	typedef struct {
		uint32_t biSize;          // 信息头大小
		int32_t  biWidth;         // 宽度
		int32_t  biHeight;        // 高度（负数表示顶行优先）
		uint16_t biPlanes;        // 平面数，1
		uint16_t biBitCount;      // 位深，24（RGB）
		uint32_t biCompression;   // 压缩方式，0（不压缩）
		uint32_t biSizeImage;     // 像素数据大小
		int32_t  biXPelsPerMeter; // 水平分辨率，0
		int32_t  biYPelsPerMeter; // 垂直分辨率，0（原代码笔误，已修正）
		uint32_t biClrUsed;       // 颜色表使用数，0
		uint32_t biClrImportant;  // 重要颜色数，0
	} BITMAPINFOHEADER;
#pragma pack(pop) // 恢复默认对齐

	// 验证结构体大小（必须符合BMP标准）
	static_assert(sizeof(BITMAPFILEHEADER) == 14, "BMP文件头大小必须为14字节");
	static_assert(sizeof(BITMAPINFOHEADER) == 40, "BMP信息头大小必须为40字节");

	// 计算像素数据大小（每行按4字节对齐）
	int row_size = (width * 3 + 3) & ~3;
	int img_size = row_size * height;

	// 填充文件头（显式处理小端序，确保跨平台兼容）
	BITMAPFILEHEADER bmp_file_header = { 0 };
	bmp_file_header.bfType = 0x4D42; // "BM"，小端序下存储为0x424D，读取为0x4D42
	bmp_file_header.bfSize = static_cast<uint32_t>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + img_size);
	bmp_file_header.bfReserved1 = 0;
	bmp_file_header.bfReserved2 = 0;
	bmp_file_header.bfOffBits = static_cast<uint32_t>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

	// 填充信息头
	BITMAPINFOHEADER bmp_info_header = { 0 };
	bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
	bmp_info_header.biWidth = width;
	bmp_info_header.biHeight = -height; // 负号表示顶行优先（和视频帧一致）
	bmp_info_header.biPlanes = 1;
	bmp_info_header.biBitCount = 24;
	bmp_info_header.biCompression = 0; // BI_RGB，不压缩
	bmp_info_header.biSizeImage = static_cast<uint32_t>(img_size);
	bmp_info_header.biXPelsPerMeter = 0;
	bmp_info_header.biYPelsPerMeter = 0;
	bmp_info_header.biClrUsed = 0;
	bmp_info_header.biClrImportant = 0;

	// 打开文件
	FILE* fp = nullptr;
#if defined(_MSC_VER)
	errno_t open_err = fopen_s(&fp, output_path.c_str(), "wb");
	if (open_err != 0 || !fp) {
		std::cerr << "无法打开文件: " << output_path << std::endl;
		return false;
	}
#else
	fp = fopen(output_path.c_str(), "wb");
	if (!fp) {
		std::cerr << "无法打开文件: " << output_path << std::endl;
		return false;
	}
#endif

	// 写入头信息（强制按1字节写入，避免对齐问题）
	fwrite(&bmp_file_header, 1, sizeof(BITMAPFILEHEADER), fp);
	fwrite(&bmp_info_header, 1, sizeof(BITMAPINFOHEADER), fp);

	// 写入像素数据（按行对齐，修复原代码指针偏移问题）
	const uint8_t* src_ptr = rgb_data;
	for (int y = 0; y < height; y++) {
		// 写入当前行有效像素数据
		fwrite(src_ptr, 3, width, fp);
		// 补充对齐字节（如果需要）
		int padding = row_size - width * 3;
		if (padding > 0) {
			static const uint8_t pad[4] = { 0 };
			fwrite(pad, 1, padding, fp);
		}
		src_ptr += width * 3; // 下一行RGB数据
	}

	// 安全关闭文件
	if (fp) {
		fclose(fp);
		fp = nullptr;
	}

	std::cout << "第一帧已保存为: " << output_path << std::endl;
	return true;
}

// 获取视频第一帧并保存
bool AvWorker::GetVideoFirstFrame(const std::string& input_url, const std::string& output_bmp, bool is_rtsp) {
	// 1. 初始化FFmpeg
	avformat_network_init(); // 网络流需要初始化网络模块

	AVFormatContext* fmt_ctx = nullptr;
	AVDictionary* options = nullptr;

	// 针对RTSP流设置参数（可选）
	if (is_rtsp) {
		av_dict_set(&options, "rtsp_transport", "tcp", 0); // TCP传输更稳定
		av_dict_set(&options, "stimeout", "5000000", 0);   // 5秒超时
		av_dict_set(&options, "max_delay", "1000000", 0);  // 1秒最大延迟
	}

	// 2. 打开输入文件/流
	int ret = avformat_open_input(&fmt_ctx, input_url.c_str(), nullptr, &options);
	av_dict_free(&options); // 释放参数字典
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "打开输入失败: " << err_buf << std::endl;
		return false;
	}

	// 3. 获取流信息
	if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
		std::cerr << "Get stream info failed" << std::endl;
		avformat_close_input(&fmt_ctx);
		return false;
	}

	// 4. 找到视频流
	int video_stream_idx = -1;
	AVCodecParameters* codec_par = nullptr;
	for (int i = 0; i < fmt_ctx->nb_streams; i++) {
		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_idx = i;
			codec_par = fmt_ctx->streams[i]->codecpar;
			break;
		}
	}
	if (video_stream_idx < 0) {
		std::cerr << "未找到视频流" << std::endl;
		avformat_close_input(&fmt_ctx);
		return false;
	}

	// 5. 查找解码器并初始化
	const AVCodec* codec = avcodec_find_decoder(codec_par->codec_id);
	if (!codec) {
		std::cerr << "找不到解码器" << std::endl;
		avformat_close_input(&fmt_ctx);
		return false;
	}

	AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		std::cerr << "分配解码器上下文失败" << std::endl;
		avformat_close_input(&fmt_ctx);
		return false;
	}

	avcodec_parameters_to_context(codec_ctx, codec_par);
	if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
		std::cerr << "open codec fair" << std::endl;
		avcodec_free_context(&codec_ctx);
		avformat_close_input(&fmt_ctx);
		return false;
	}

	// 6. 初始化缩放上下文（转换为RGB24格式）
	SwsContext* sws_ctx = sws_getContext(
		codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
		codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, // BGR24和RGB24仅通道顺序不同，BMP用BGR更方便
		SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!sws_ctx) {
		std::cerr << "create sws_ctx fair" << std::endl;
		avcodec_free_context(&codec_ctx);
		avformat_close_input(&fmt_ctx);
		return false;
	}

	// 7. 分配帧缓存
	AVFrame* frame = av_frame_alloc();       // 原始视频帧
	AVFrame* frame_rgb = av_frame_alloc();   // 转换后的RGB帧
	if (!frame || !frame_rgb) {
		std::cerr << "alloc frame fair" << std::endl;
		sws_freeContext(sws_ctx);
		avcodec_free_context(&codec_ctx);
		avformat_close_input(&fmt_ctx);
		return false;
	}

	// 分配RGB像素缓冲区
	int rgb_buffer_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height, 1);
	uint8_t* rgb_buffer = (uint8_t*)av_malloc(rgb_buffer_size);
	if (!rgb_buffer) {
		std::cerr << "alloc rgb buff fair" << std::endl;
		av_frame_free(&frame);
		av_frame_free(&frame_rgb);
		sws_freeContext(sws_ctx);
		avcodec_free_context(&codec_ctx);
		avformat_close_input(&fmt_ctx);
		return false;
	}
	av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize,
		rgb_buffer, AV_PIX_FMT_BGR24,
		codec_ctx->width, codec_ctx->height, 1);

	// 8. 读取并解码第一帧
	AVPacket pkt = { 0 };
	bool got_first_frame = false;
	while (av_read_frame(fmt_ctx, &pkt) >= 0 && !got_first_frame) {
		if (pkt.stream_index != video_stream_idx) {
			av_packet_unref(&pkt);
			continue;
		}

		// 发送数据包到解码器
		ret = avcodec_send_packet(codec_ctx, &pkt);
		if (ret < 0) {
			std::cerr << "send packet fair" << std::endl;
			av_packet_unref(&pkt);
			continue;
		}

		// 接收解码后的帧
		ret = avcodec_receive_frame(codec_ctx, frame);
		if (ret == 0) { // 成功解码一帧
			// 转换为BGR24格式
			sws_scale(sws_ctx,
				(const uint8_t* const*)frame->data, frame->linesize,
				0, codec_ctx->height,
				frame_rgb->data, frame_rgb->linesize);

			// 保存为BMP图片
			SaveFrameToBmp(rgb_buffer, codec_ctx->width, codec_ctx->height, output_bmp);
			got_first_frame = true; // 拿到第一帧后退出循环
		}

		av_packet_unref(&pkt);
	}

	if (!got_first_frame) {
		std::cerr << "got_first_frame is NULL" << std::endl;
	}

	// 9. 释放所有资源
	av_free(rgb_buffer);
	av_frame_free(&frame);
	av_frame_free(&frame_rgb);
	sws_freeContext(sws_ctx);
	avcodec_free_context(&codec_ctx);
	avformat_close_input(&fmt_ctx);
	avformat_network_deinit(); // 释放网络资源（如果是RTSP流）

	return got_first_frame;
}


bool AvWorker::SpliceAV(const std::string& input_url1, const std::string& input_url2, const std::string& output_url, bool is_rtsp)
{
	// 1. 初始化FFmpeg网络模块（本地文件也建议保留，兼容RTSP场景）
	avformat_network_init();

	// 资源初始化（所有资源默认置空，便于统一释放）
	AVFormatContext* fmt_ctx1 = nullptr;
	AVFormatContext* fmt_ctx2 = nullptr;
	AVFormatContext* out_fmt_ctx = nullptr;
	AVDictionary* options = nullptr;
	int ret = 0;
	bool is_success = false; // 标记整体操作是否成功

	// 针对RTSP流设置参数（本地文件会自动忽略）
	if (is_rtsp) {
		av_dict_set(&options, "rtsp_transport", "tcp", 0);
		av_dict_set(&options, "stimeout", "5000000", 0);
		av_dict_set(&options, "max_delay", "1000000", 0);
	}

	// 2. 打开第一个输入文件/流（错误处理：失败则直接释放资源返回）
	ret = avformat_open_input(&fmt_ctx1, input_url1.c_str(), nullptr, &options);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "[error] open_input 1 fair index: " << input_url1 << "-" << err_buf << std::endl;
		av_dict_free(&options);
		avformat_network_deinit();
		return false;
	}

	// 3. 打开第二个输入文件/流（错误处理：失败则释放第一个输入资源）
	ret = avformat_open_input(&fmt_ctx2, input_url2.c_str(), nullptr, &options);
	if (ret < 0){
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "[error]open_input 2 fair index:" << input_url2 << "-" << err_buf << std::endl;
		av_dict_free(&options);
		avformat_close_input(&fmt_ctx1);
		avformat_network_deinit();
		return false;
	}
	av_dict_free(&options);
	options = nullptr;

	// 4. 获取第一个输入流信息
	ret = avformat_find_stream_info(fmt_ctx1, nullptr);
	if (ret < 0) {
		std::cerr << "[error]find_stream_info 1 fair index:" << input_url1 << std::endl;
		avformat_close_input(&fmt_ctx1);
		avformat_close_input(&fmt_ctx2);
		avformat_network_deinit();
		return false;
	}

	// 获取第二个输入流信息
	ret = avformat_find_stream_info(fmt_ctx2, nullptr);
	if (ret < 0) {
		std::cerr << "[error]find_stream_info 2 fair index:" << input_url2 << std::endl;
		avformat_close_input(&fmt_ctx1);
		avformat_close_input(&fmt_ctx2);
		avformat_network_deinit();
		return false;
	}

	// 打印输入流信息（调试用）
	std::cout << "[info] Input 1 streams count: " << fmt_ctx1->nb_streams << std::endl;
	std::cout << "[info] Input 2 streams count: " << fmt_ctx2->nb_streams << std::endl;

	// 5. 创建输出上下文
	ret = avformat_alloc_output_context2(&out_fmt_ctx, nullptr, nullptr, output_url.c_str());
	if (!out_fmt_ctx) {
		std::cerr << "[error]craete output stream fair index:" << output_url << std::endl;
		avformat_close_input(&fmt_ctx1);
		avformat_close_input(&fmt_ctx2);
		avformat_network_deinit();
		return false;
	}

	// 6. 复制流信息到输出上下文
	bool stream_copy_ok = true;
	for (int i = 0; i < fmt_ctx1->nb_streams && stream_copy_ok; i++) {
		AVStream* in_stream = fmt_ctx1->streams[i];
		AVStream* out_stream = avformat_new_stream(out_fmt_ctx, nullptr);

		if (!out_stream) {
			std::cerr << "[error]create output stream fair index:" << i << std::endl;
			stream_copy_ok = false;
			break;
		}

		// 复制流参数
		ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
		if (ret < 0) {
			char err_buf[1024] = { 0 };
			av_strerror(ret, err_buf, sizeof(err_buf));
			std::cerr << "[error]copy stream fair index:" << i << " - " << err_buf << std::endl;
			stream_copy_ok = false;
			break;
		}
		out_stream->codecpar->codec_tag = 0;

		// 关键：把输入流的 time_base 同步到输出流，避免后续 rescale 使用错误 time_base
		out_stream->time_base = in_stream->time_base;
		out_stream->avg_frame_rate = in_stream->avg_frame_rate;
		out_stream->r_frame_rate = in_stream->r_frame_rate;
		out_stream->sample_aspect_ratio = in_stream->sample_aspect_ratio;
	}

	// 流复制失败的错误处理
	if (!stream_copy_ok) {
		avformat_free_context(out_fmt_ctx);
		avformat_close_input(&fmt_ctx1);
		avformat_close_input(&fmt_ctx2);
		avformat_network_deinit();
		return false;
	}

	// 7. 打开输出文件IO
	if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		ret = avio_open(&out_fmt_ctx->pb, output_url.c_str(), AVIO_FLAG_WRITE);
		if (ret < 0) {
			char err_buf[1024] = { 0 };
			av_strerror(ret, err_buf, sizeof(err_buf));
			std::cerr << "[error]open  output fair url:" << output_url << " - " << err_buf << std::endl;
			avformat_free_context(out_fmt_ctx);
			avformat_close_input(&fmt_ctx1);
			avformat_close_input(&fmt_ctx2);
			avformat_network_deinit();
			return false;
		}
	}

	//  写入文件头
	ret = avformat_write_header(out_fmt_ctx, nullptr);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "[error]write header fair" << output_url << "-" << err_buf << std::endl;

		// 释放输出资源
		if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
			avio_closep(&out_fmt_ctx->pb);
		}
		avformat_free_context(out_fmt_ctx);

		// 释放输入资源
		avformat_close_input(&fmt_ctx1);
		avformat_close_input(&fmt_ctx2);
		avformat_network_deinit();
		return false;
	}

	// 处理第一个视频的音视频帧
	AVPacket pkt = {};
	av_packet_unref(&pkt);

	// 定义AVRounding枚举变量，解决类型不兼容问题
	AVRounding rounding = static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);

	// 关键：按“每个输出流”累计结束时间戳 + 最后一帧的DTS/PTS（用于单调性校验）
	const int out_nb_streams = static_cast<int>(out_fmt_ctx->nb_streams);
	std::vector<int64_t> out_stream_end_ts(out_nb_streams, 0);        // 拼接偏移量
	std::vector<int64_t> out_stream_last_dts(out_nb_streams, -1);     // 最后一帧的DTS
	std::vector<int64_t> out_stream_last_pts(out_nb_streams, -1);     // 最后一帧的PTS
	std::vector<bool> out_stream_has_ts(out_nb_streams, false);

	auto rescale_ts = [&](int64_t ts, const AVRational& in_tb, const AVRational& out_tb) -> int64_t {
		if (ts == AV_NOPTS_VALUE) {
			return AV_NOPTS_VALUE;
		}
		return av_rescale_q_rnd(ts, in_tb, out_tb, rounding);
	};

	// 关键：时间戳单调性修正函数
	auto fix_timestamp_monotonic = [&](int stream_idx, int64_t& pts, int64_t& dts) {
		if (stream_idx < 0 || stream_idx >= out_nb_streams) return;

		// 修正DTS（必须严格递增）
		if (dts != AV_NOPTS_VALUE) {
			if (out_stream_last_dts[stream_idx] >= 0) {
				if (dts <= out_stream_last_dts[stream_idx]) {
					dts = out_stream_last_dts[stream_idx] + 1;
				}
			}
			out_stream_last_dts[stream_idx] = dts;
		}

		// 修正PTS（PTS >= DTS 且递增）
		if (pts != AV_NOPTS_VALUE) {
			if (out_stream_last_pts[stream_idx] >= 0) {
				if (pts <= out_stream_last_pts[stream_idx]) {
					pts = out_stream_last_pts[stream_idx] + 1;
				}
			}
			// 确保PTS >= DTS
			if (dts != AV_NOPTS_VALUE && pts < dts) {
				pts = dts;
			}
			out_stream_last_pts[stream_idx] = pts;
		}
	};

	bool first_video_write_ok = true;
	int first_video_frame_count = 0; // 统计第一个视频写入帧数
	while (first_video_write_ok && av_read_frame(fmt_ctx1, &pkt) >= 0) {
		const int out_index = pkt.stream_index;
		if (out_index < 0 || out_index >= out_nb_streams) {
			av_packet_unref(&pkt);
			pkt = {};
			continue;
		}

		AVStream* in_stream = fmt_ctx1->streams[pkt.stream_index];
		AVStream* out_stream = out_fmt_ctx->streams[out_index];
		if (!out_stream) {
			av_packet_unref(&pkt);
			pkt = {};
			continue;
		}

		// 时基转换
		pkt.pts = rescale_ts(pkt.pts, in_stream->time_base, out_stream->time_base);
		pkt.dts = rescale_ts(pkt.dts, in_stream->time_base, out_stream->time_base);
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;

		// 修正时间戳单调性（第一个视频也要做）
		fix_timestamp_monotonic(out_index, pkt.pts, pkt.dts);

		ret = av_interleaved_write_frame(out_fmt_ctx, &pkt);
		if (ret < 0) {
			char err_buf[1024] = { 0 };
			av_strerror(ret, err_buf, sizeof(err_buf));
			std::cerr << "[error] write first frame failed, stream: " << out_index << " - " << err_buf << std::endl;
			first_video_write_ok = false;
			av_packet_unref(&pkt);
			pkt = {};
			break;
		}

		first_video_frame_count++;

		// 累计每个输出流的结束时间戳（用修正后的DTS）
		int64_t ts_for_end = (pkt.dts != AV_NOPTS_VALUE) ? pkt.dts : pkt.pts;
		if (ts_for_end != AV_NOPTS_VALUE) {
			int64_t end_ts = ts_for_end + ((pkt.duration > 0) ? pkt.duration : 1);
			if (!out_stream_has_ts[out_index] || end_ts > out_stream_end_ts[out_index]) {
				out_stream_end_ts[out_index] = end_ts;
				out_stream_has_ts[out_index] = true;
			}
		}

		av_packet_unref(&pkt);
		pkt = {};
	}

	std::cout << "[info] First video frame count: " << first_video_frame_count << std::endl;

	// 第一个视频写入失败的错误处理
	if (!first_video_write_ok) {
		av_write_trailer(out_fmt_ctx);
		if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
			avio_closep(&out_fmt_ctx->pb);
		}
		avformat_free_context(out_fmt_ctx);
		avformat_close_input(&fmt_ctx1);
		avformat_close_input(&fmt_ctx2);
		avformat_network_deinit();
		return false;
	}

	// 10. 处理第二个视频的音视频帧
	pkt = {};
	av_packet_unref(&pkt);

	// 流映射（按类型匹配）
	std::vector<int> stream_mapping2(static_cast<int>(fmt_ctx2->nb_streams), -1);
	std::vector<int> video_stream_idx;
	std::vector<int> audio_stream_idx;
	for (int j = 0; j < out_nb_streams; ++j) {
		AVStream* out = out_fmt_ctx->streams[j];
		if (out->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_idx.push_back(j);
		}
		else if (out->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_stream_idx.push_back(j);
		}
	}

	int video_idx = 0, audio_idx = 0;
	for (int i = 0; i < static_cast<int>(fmt_ctx2->nb_streams); ++i) {
		AVStream* in2 = fmt_ctx2->streams[i];
		if (in2->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && video_idx < video_stream_idx.size()) {
			stream_mapping2[i] = video_stream_idx[video_idx++];
		}
		else if (in2->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audio_idx < audio_stream_idx.size()) {
			stream_mapping2[i] = audio_stream_idx[audio_idx++];
		}
		else {
			stream_mapping2[i] = -1;
		}
	}

	// 打印映射信息
	std::cout << "[info] Stream mapping for input 2: " << std::endl;
	for (int i = 0; i < stream_mapping2.size(); ++i) {
		std::cout << "  Input2 stream " << i << " -> Output stream " << stream_mapping2[i] << std::endl;
	}

	// 第二个视频的首帧偏移（按流独立计算）
	std::vector<int64_t> in2_first_pts(static_cast<int>(fmt_ctx2->nb_streams), AV_NOPTS_VALUE);
	std::vector<int64_t> in2_first_dts(static_cast<int>(fmt_ctx2->nb_streams), AV_NOPTS_VALUE);

	bool second_video_write_ok = true;
	int second_video_frame_count = 0;

	while (second_video_write_ok && av_read_frame(fmt_ctx2, &pkt) >= 0) {
		const int in_index = pkt.stream_index;
		if (in_index < 0 || in_index >= static_cast<int>(stream_mapping2.size())) {
			av_packet_unref(&pkt);
			pkt = {};
			continue;
		}
		const int out_index = stream_mapping2[in_index];
		if (out_index < 0 || out_index >= out_nb_streams) {
			av_packet_unref(&pkt);
			pkt = {};
			continue;
		}

		AVStream* in_stream = fmt_ctx2->streams[in_index];
		AVStream* out_stream = out_fmt_ctx->streams[out_index];
		if (!out_stream) {
			av_packet_unref(&pkt);
			pkt = {};
			second_video_write_ok = false;
			break;
		}

		// 转换到输出时基
		int64_t new_pts = rescale_ts(pkt.pts, in_stream->time_base, out_stream->time_base);
		int64_t new_dts = rescale_ts(pkt.dts, in_stream->time_base, out_stream->time_base);
		int64_t new_duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);

		// 第二个视频自身起点归零（按流独立）
		if (new_pts != AV_NOPTS_VALUE && in2_first_pts[in_index] == AV_NOPTS_VALUE) {
			in2_first_pts[in_index] = new_pts;
		}
		if (new_dts != AV_NOPTS_VALUE && in2_first_dts[in_index] == AV_NOPTS_VALUE) {
			in2_first_dts[in_index] = new_dts;
		}
		if (new_pts != AV_NOPTS_VALUE && in2_first_pts[in_index] != AV_NOPTS_VALUE) {
			new_pts -= in2_first_pts[in_index];
		}
		if (new_dts != AV_NOPTS_VALUE && in2_first_dts[in_index] != AV_NOPTS_VALUE) {
			new_dts -= in2_first_dts[in_index];
		}

		// 加上第一个视频的结束偏移（核心修复：按输出流独立偏移）
		int64_t offset = out_stream_end_ts[out_index];
		if (new_pts != AV_NOPTS_VALUE) new_pts += offset;
		if (new_dts != AV_NOPTS_VALUE) new_dts += offset;

		// 强制修正时间戳单调性（解决核心错误）
		fix_timestamp_monotonic(out_index, new_pts, new_dts);

		// 赋值给packet
		pkt.stream_index = out_index;
		pkt.pts = new_pts;
		pkt.dts = new_dts;
		pkt.duration = new_duration;
		pkt.pos = -1;

		// 写入帧（忽略单帧错误，继续处理）
		ret = av_interleaved_write_frame(out_fmt_ctx, &pkt);
		if (ret < 0) {
			char err_buf[1024] = { 0 };
			av_strerror(ret, err_buf, sizeof(err_buf));
			std::cerr << "[warning] write second frame failed (ignored), in_stream: " << in_index
				<< " -> out_stream: " << out_index << " - " << err_buf << std::endl;
			av_packet_unref(&pkt);
			pkt = {};
			continue;
		}

		second_video_frame_count++;

		// 更新结束时间戳
		int64_t ts_for_end = (pkt.dts != AV_NOPTS_VALUE) ? pkt.dts : pkt.pts;
		if (ts_for_end != AV_NOPTS_VALUE) {
			int64_t end_ts = ts_for_end + ((pkt.duration > 0) ? pkt.duration : 1);
			if (end_ts > out_stream_end_ts[out_index]) {
				out_stream_end_ts[out_index] = end_ts;
			}
		}

		av_packet_unref(&pkt);
		pkt = {};
	}

	std::cout << "[info] Second video frame count: " << second_video_frame_count << std::endl;

	// 刷新缓冲区
	if (out_fmt_ctx && out_fmt_ctx->pb) {
		avio_flush(out_fmt_ctx->pb);
	}

	// 写入文件尾
	ret = av_write_trailer(out_fmt_ctx);
	if (ret < 0) {
		char err_buf[1024] = { 0 };
		av_strerror(ret, err_buf, sizeof(err_buf));
		std::cerr << "[错误] 写入文件尾失败: " << output_url << " - " << err_buf << std::endl;
	}
	else {
		std::cout << "[成功] 视频拼接完成，输出文件: " << output_url << std::endl;
		is_success = true;
	}

	// 释放资源
	if (fmt_ctx1) avformat_close_input(&fmt_ctx1);
	if (fmt_ctx2) avformat_close_input(&fmt_ctx2);

	if (out_fmt_ctx) {
		if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
			avio_closep(&out_fmt_ctx->pb);
		}
		avformat_free_context(out_fmt_ctx);
	}

	avformat_network_deinit();

	return is_success;
}

extern "C" OPENCVFFMPEGTOOLS_API void* AvWorker_Create()
{
	return new AvWorker();
}

extern "C" OPENCVFFMPEGTOOLS_API void AvWorker_Destroy(void* worker)
{
	delete static_cast<AvWorker*>(worker);
}

extern "C" OPENCVFFMPEGTOOLS_API bool AvWorker_GetVideoFirstFrame(void* worker, const char* input_url, const char* output_bmp, bool is_rtsp)
{
	if (!worker || !input_url || !output_bmp) {
		return false;
	}
	return static_cast<AvWorker*>(worker)->GetVideoFirstFrame(input_url, output_bmp, is_rtsp);
}

extern "C" OPENCVFFMPEGTOOLS_API bool AvWorker_SpliceAV(void* worker, const char* input_url1, const char* input_url2, const char* output_url, bool is_rtsp)
{
	if (!worker || !input_url1 || !input_url2 || !output_url) {
		return false;
	}
	return static_cast<AvWorker*>(worker)->SpliceAV(input_url1, input_url2, output_url, is_rtsp);
}

extern "C" OPENCVFFMPEGTOOLS_API bool AvWorker_resize_video(void* worker, const char* input_url, const char* output_url, int dst_width, int dst_height)
{
	if (!worker || !input_url  || !output_url) {
		return false;
	}
	return static_cast<AvWorker*>(worker)->resize_video(input_url,  output_url, dst_width,dst_height);
}

