#include "pch.h"
#include "COpenCVTools.h"

COpenCVTools::COpenCVTools()
{
	return;
}

void COpenCVTools::test()
{
	cv::Mat a;
}

cv::Mat COpenCVTools::AVFrameToCVMat(AVFrame* yuv420Frame)
{
	if (!yuv420Frame || yuv420Frame->width <= 0 || yuv420Frame->height <= 0) {
		return cv::Mat(); // 返回空Mat
	}

	// 得到AVFrame信息
	int srcW = yuv420Frame->width;
	int srcH = yuv420Frame->height;
	AVPixelFormat srcFormat = (AVPixelFormat)yuv420Frame->format;
	
	// 检查输入格式是否有效
	if (srcFormat < 0 || srcFormat >= AV_PIX_FMT_NB) {
		return cv::Mat();
	}

	// 直接创建3通道BGR Mat
	cv::Mat mat(srcH, srcW, CV_8UC3);
	
	// 创建SwsContext：从任意格式转换为BGR24
	SwsContext* swsCtx = sws_getContext(
		srcW, srcH, srcFormat,
		srcW, srcH, AV_PIX_FMT_BGR24,
		SWS_BICUBIC, NULL, NULL, NULL);

	if (!swsCtx) {
		return cv::Mat(); // SwsContext创建失败
	}

	// 设置输出数据指针和步长（直接使用Mat的数据）
	uint8_t* dst_data[4] = { mat.data, nullptr, nullptr, nullptr };
	int dst_linesize[4] = { mat.cols * 3, 0, 0, 0 };

	// 执行格式转换
	sws_scale(swsCtx,
		(const uint8_t* const*)yuv420Frame->data, yuv420Frame->linesize,
		0, srcH,
		dst_data, dst_linesize);

	// 释放
	sws_freeContext(swsCtx);

	return mat;
}

AVFrame* COpenCVTools::CVMatToAVFrame(cv::Mat& inMat)
{
	if (inMat.empty() || inMat.cols <= 0 || inMat.rows <= 0) {
		return nullptr;
	}

	// 确保输入是3通道BGR格式
	cv::Mat bgrMat;
	if (inMat.channels() == 1) {
		// 单通道转3通道（灰度转BGR）
		cv::cvtColor(inMat, bgrMat, cv::COLOR_GRAY2BGR);
	} else if (inMat.channels() == 4) {
		// BGRA转BGR
		cv::cvtColor(inMat, bgrMat, cv::COLOR_BGRA2BGR);
	} else if (inMat.channels() == 3) {
		bgrMat = inMat;
	} else {
		// 不支持的通道数
		return nullptr;
	}

	int width = bgrMat.cols;
	int height = bgrMat.rows;

	// 创建AVFrame填充参数 注：调用者释放该frame
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		return nullptr;
	}
	frame->width = width;
	frame->height = height;
	frame->format = AV_PIX_FMT_YUV420P;

	// 初始化AVFrame内部空间
	int ret = av_frame_get_buffer(frame, 32);
	if (ret < 0) {
		av_frame_free(&frame);
		return nullptr;
	}
	ret = av_frame_make_writable(frame);
	if (ret < 0) {
		av_frame_free(&frame);
		return nullptr;
	}

	// 使用SwsContext进行转换
	SwsContext* swsCtx = sws_getContext(
		width, height, AV_PIX_FMT_BGR24,
		width, height, AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL);

	if (!swsCtx) {
		av_frame_free(&frame);
		return nullptr;
	}

	// 设置输入数据指针
	uint8_t* src_data[4] = { bgrMat.data, nullptr, nullptr, nullptr };
	int src_linesize[4] = { width * 3, 0, 0, 0 };

	// 执行转换
	sws_scale(swsCtx,
		(const uint8_t* const*)src_data, src_linesize,
		0, height,
		frame->data, frame->linesize);

	// 清理
	sws_freeContext(swsCtx);

	return frame;
}
