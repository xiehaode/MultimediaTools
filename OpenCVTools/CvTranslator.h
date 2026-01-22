/*****************************************************************//**
 * \file   CvTranslator.h
 * \brief  基于opencv的变换
 * 
 * \author 28026
 * \date   January 2026
 *********************************************************************/
#pragma once
/*
#ifdef OPENCVTOOLS_EXPORTS
#define OPENCVFFMPEGTOOLS_API __declspec(dllexport)
#else
#define OPENCVTOOLS_API __declspec(dllimport)
#endif
*/

#include "OpenCVFFMpegTools.h"

class OPENCVFFMPEGTOOLS_API CvTranslator
{
public:
	explicit CvTranslator();
	/**
	* .
	* 
	* \param frame 源文件
	* \return 灰度图像
	*/
	cv::Mat grayImage(const cv::Mat & frame);
	/**
	* @brief 优化版油画效果模拟（无任何xphoto依赖）
	* @param src 输入8位彩色BGR图像
	* @param radius 空间滤波半径（影响纹理粗细）
	* @param sigma_color 颜色sigma（影响颜色融合度）
	* @return 油画效果图像
	*/
	cv::Mat customOilPaintApprox(const cv::Mat& src, int radius, double sigma_color);
	/**
	* @brief 油画滤镜实现函数（纯自定义实现，无xphoto依赖）
	* @param src 输入图像（cv::Mat，支持8位彩色图像）
	* @param radius 油画效果半径（空间滤波参数）
	* @param sigma_color 颜色空间滤波sigma（控制颜色平滑度）
	* @return 处理后的油画效果图像（cv::Mat）
	*/
	cv::Mat applyOilPainting(const cv::Mat& src, int radius = 5, double sigma_color = 8.0);
	/**
	* .
	*
	* @param src 源文件
    * @param mosaicRegion 需要的区域
	* @param cellSize 马赛克块的大小
	* @return 目标mat
	*/
	cv::Mat applyMosaic(const cv::Mat &src, const cv::Rect &mosaicRegion, int cellSize);
	/**
	 * .
	 * 
	 * \param imageSource 源文件
 	 * \return 毛玻璃效果的mat 
	 */
	cv::Mat FrostedGlass(const cv::Mat &imageSource);
	/**
	 * @brief. 文字水印
	 * 
	 * \param src
	 * \param text
	 * \return 
	 */
	cv::Mat addTextWatermark(const cv::Mat& src, const std::string& text);


	/**
	 * @brief 磨皮.
	 * 
	 * \param inputImage
	 * \return 
	 */
	cv::Mat simpleSkinSmoothing(const cv::Mat& inputImage);
	/**
	 * @brief 美白.
	 * 
	 * \param src
	 * \return 
	 */
	cv::Mat Whitening(const cv::Mat& src);
	/**
	 * @brief 美白2.
	 *
	 * \param src
	 * \return
	 */
	cv::Mat Whitening2(const cv::Mat &src);
	/**
	 * @brief 水印.
	 * 
	 * \param src
	 * \param text
	 * \param pos
	 * \param fontScale
	 * \param color
	 * \param thickness
	 * \param lineType
	 * \return 
	 */
	cv::Mat addTextWatermark(const cv::Mat& src, const std::string& text,
		const cv::Point& pos = cv::Point(50, 50),
		double fontScale = 1.0,
		cv::Scalar color = cv::Scalar(0, 0, 255),  // BGR格式，默认红色
		int thickness = 2,
		int lineType = cv::LINE_AA);
	/**
	 * @brief 图像颜色反转.
	 * 
	 * \param src
	 * \return 
	 */
	cv::Mat invertImage(cv::Mat src);
};

