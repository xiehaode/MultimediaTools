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
	// 得到AVFrame信息
	int srcW = yuv420Frame->width;
	int srcH = yuv420Frame->height;
	SwsContext* swsCtx = sws_getContext(
		srcW, srcH, (AVPixelFormat)yuv420Frame->format,
		srcW, srcH, (AVPixelFormat)AV_PIX_FMT_BGR24,
		SWS_BICUBIC, NULL, NULL, NULL);

	// 生成Mat对象
	cv::Mat mat;
	mat.create(cv::Size(srcW, srcH), CV_8UC3);

	// 格式转换，直接填充Mat的数据data
	AVFrame* bgr24Frame = av_frame_alloc();
	av_image_fill_arrays(bgr24Frame->data, bgr24Frame->linesize,
		(uint8_t*)mat.data, (AVPixelFormat)AV_PIX_FMT_BGR24, srcW, srcH, 1);
	sws_scale(swsCtx,
		(const uint8_t* const*)yuv420Frame->data, yuv420Frame->linesize,
		0, srcH,
		bgr24Frame->data, bgr24Frame->linesize);

	// 释放
	av_frame_free(&bgr24Frame);
	sws_freeContext(swsCtx);

	return mat;
}

AVFrame* COpenCVTools::CVMatToAVFrame(cv::Mat& inMat)
{
	// 得到Mat信息
	AVPixelFormat dstFormat = AV_PIX_FMT_YUV420P;
	int width = inMat.cols;
	int height = inMat.rows;

	// 创建AVFrame填充参数 注：调用者释放该frame
	AVFrame* frame = av_frame_alloc();
	frame->width = width;
	frame->height = height;
	frame->format = dstFormat;

	// 初始化AVFrame内部空间
	int ret = av_frame_get_buffer(frame, 32);
	if (ret < 0) {
		return nullptr;
	}
	ret = av_frame_make_writable(frame);
	if (ret < 0) {
		return nullptr;
	}

	// 转换颜色空间为YUV420
	cv::cvtColor(inMat, inMat, cv::COLOR_BGR2YUV_I420);

	// 按YUV420格式，设置数据地址
	int frame_size = width * height;
	unsigned char* data = inMat.data;
	memcpy(frame->data[0], data, frame_size);
	memcpy(frame->data[1], data + frame_size, frame_size / 4);
	memcpy(frame->data[2], data + frame_size * 5 / 4, frame_size / 4);

	return frame;
}
