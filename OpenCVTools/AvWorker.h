#pragma once

#include "OpenCVFFMpegTools.h"

class OPENCVFFMPEGTOOLS_API AvWorker
{
public :
	explicit AvWorker();
	void initAv();
	void finishAv();
	bool GetVideoFirstFrame(const std::string& input_url, const std::string& output_bmp, bool is_rtsp = false);
	bool SaveFrameToBmp(const uint8_t* rgb_data, int width, int height, const std::string& output_path);
};

