#define  _CRT_SECURE_NO_WARNINGS 1
#include "pch.h"
#include "AvWorker.h"



AvWorker::AvWorker()
{

}

void AvWorker::initAv()
{
}

void AvWorker::finishAv()
{
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
