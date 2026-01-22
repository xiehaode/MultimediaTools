#pragma once

// COpenCVTools：单独暴露 OpenCV/FFmpeg 相关 C++ 接口。
// 说明：如果外部只想使用纯 C 接口（不引入 OpenCV 头），请只包含 `OpenCVFFMpegTools.h`。

#include "OpenCVFFMpegTools.h"

#ifdef __cplusplus
#include <opencv2/opencv.hpp>

struct AVFrame;

class OPENCVFFMPEGTOOLS_API COpenCVTools {
public:
	COpenCVTools();
	void test();
	cv::Mat AVFrameToCVMat(AVFrame* yuv420Frame);
	AVFrame* CVMatToAVFrame(cv::Mat& inMat);
};
#endif
