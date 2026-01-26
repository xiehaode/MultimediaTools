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
	/**
	 * @brief 合并视频.
	 * 
	 * \param input_url1
	 * \param input_url2
	 * \param output_url
	 * \param is_rtsp
	 * \return 
	 */
	bool SpliceAV(const std::string& input_url1, const std::string& input_url2, const std::string& output_url, bool is_rtsp);
	/**
	 * 重新设置分辨率.
	 * 
	 * \param input_path
	 * \param output_path
	 * \param dst_width
	 * \param dst_height
	 * \return 
	 */
	bool resize_video(const std::string& input_path, const std::string& output_path,
		int dst_width, int dst_height);
	/**
	 * @brief 获得视频第一帧.
	 * 
	 * \param input_url
	 * \param output_bmp
	 * \param is_rtsp
	 * \return 
	 */
	bool GetVideoFirstFrame(const std::string& input_url, const std::string& output_bmp, bool is_rtsp = false);
	/**
	 * @brief 保存为bmp.
	 * 
	 * \param rgb_data
	 * \param width
	 * \param height
	 * \param output_path
	 * \return 
	 */
	bool SaveFrameToBmp(const uint8_t* rgb_data, int width, int height, const std::string& output_path);
	/**
	 * @brief 分割视频.
	 * 
	 * \param input_path
	 * \param output_path
	 * \param start_seconds
	 * \param duration_seconds
	 * \return 
	 */
	int split_video(const std::string& input_path,
		const std::string& output_path,
		double start_seconds,
		double duration_seconds);
	double getDuration(const std::string& input_path);
};

