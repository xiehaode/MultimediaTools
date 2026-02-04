#pragma once


#include "OpenCVFFMpegTools.h"

#ifdef __cplusplus
#include <opencv2/opencv.hpp>

struct AVFrame;

class  COpenCVTools {
public:
	COpenCVTools();
	void test();
	cv::Mat AVFrameToCVMat(AVFrame* yuv420Frame);
	AVFrame* CVMatToAVFrame(cv::Mat& inMat);
};
#endif
