#pragma once

#include <cstdint>
#include <string>

#include "OpenCVFFMpegTools.h"

class OPENCVFFMPEGTOOLS_API AvWorker
{
public :
	explicit AvWorker();
	void initAv();
	void finishAv();

	bool SpliceAV(const std::string& input_url1, const std::string& input_url2, const std::string& output_url, bool is_rtsp);
	bool resize_video(const std::string& input_path, const std::string& output_path,
		int dst_width, int dst_height);
	bool GetVideoFirstFrame(const std::string& input_url, const std::string& output_bmp, bool is_rtsp = false);
	bool SaveFrameToBmp(const uint8_t* rgb_data, int width, int height, const std::string& output_path);
};

