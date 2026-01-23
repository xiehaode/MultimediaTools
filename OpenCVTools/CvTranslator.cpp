#include "pch.h"
#include "CvTranslator.h"

#include <exception>


CvTranslator::CvTranslator()
{
	
}

cv::Mat CvTranslator::grayImage(const cv::Mat & frame)
{
	if (frame.empty()) {
		return cv::Mat();
	}
	cv::Mat grayImage;

	
	cv::cvtColor(frame, grayImage, cv::COLOR_BGR2GRAY);

	if (grayImage.empty()) {
		return cv::Mat();
	}
	return grayImage;
}




cv::Mat CvTranslator::applyOilPainting(const cv::Mat& src, int radius, double sigma_color )
{
	
	if (src.empty()) {
		throw std::invalid_argument("src is null");
	}
	if (src.channels() != 3) {
		throw std::invalid_argument("only rgb");
	}
	if (src.depth() != CV_8U) {
		throw std::invalid_argument("only 8 bit");
	}
	if (radius < 1) {
		throw std::invalid_argument("radius must > 1");
	}
	if (sigma_color <= 0) {
		throw std::invalid_argument("sigma_color must >0");
	}

	// 直接调用自定义近似实现（移除xphoto分支）
	cv::Mat dst = customOilPaintApprox(src, radius, sigma_color);
	return dst;
}


cv::Mat CvTranslator::customOilPaintApprox(const cv::Mat& src, int radius, double sigma_color)
{
	// 步骤1：下采样到原尺寸的1/2，减少计算量（与Python版本完全对齐）
	cv::Mat small;
	cv::resize(src, small, cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);

	// 步骤2：双边滤波模拟油画纹理
	// 双边滤波参数说明：
	// d: 滤波核直径，设为9（与Python版本一致）
	// sigmaColor: 颜色空间的sigma，控制颜色相似性权重
	// sigmaSpace: 坐标空间的sigma，用radius值，控制空间距离权重
	cv::Mat blur;
	int d = 9;
	cv::bilateralFilter(small, blur, d, sigma_color, static_cast<double>(radius));

	// 步骤3：上采样回原尺寸，使用三次立方插值保证清晰度（与Python一致）
	cv::Mat result;
	cv::resize(blur, result, src.size(), 0, 0, cv::INTER_CUBIC);

	// 可选优化：增加轻微的锐化，让油画纹理更明显
	cv::Mat kernel = (cv::Mat_<float>(3, 3) << 0, -1, 0,
		-1, 5, -1,
		0, -1, 0);
	cv::filter2D(result, result, -1, kernel);

	return result;
}


cv::Mat CvTranslator::applyMosaic(const cv::Mat &src, const cv::Rect &mosaicRegion, int cellSize) {
	// 创建一个与源图像相同大小的目标图像
	cv::Mat dst = src.clone();

	// 马赛克区域的边界检查
	int startX = cv::max(0, mosaicRegion.x);
	int startY = cv::max(0, mosaicRegion.y);
	int endX = cv::min(src.cols, mosaicRegion.x + mosaicRegion.width);
	int endY = cv::min(src.rows, mosaicRegion.y + mosaicRegion.height);

	// 遍历马赛克区域
	for (int y = startY; y < endY; y += cellSize) {
		for (int x = startX; x < endX; x += cellSize) {
			// 计算马赛克块的边界
			int x1 = x;
			int y1 = y;
			int x2 = cv::min(x + cellSize, endX);
			int y2 = cv::min(y + cellSize, endY);

			// 计算马赛克块的中心点
			int centerX = (x1 + x2) / 2;
			int centerY = (y1 + y2) / 2;

			// 确保中心点在马赛克区域内
			if (centerX >= startX && centerX < endX && centerY >= startY && centerY < endY) {
				cv::Vec3b centerPixel = src.at<cv::Vec3b>(centerY, centerX);

				// 将马赛克块内的像素值设置为中心像素值
				for (int i = y1; i < y2; ++i) {
					for (int j = x1; j < x2; ++j) {
						dst.at<cv::Vec3b>(i, j) = centerPixel;
					}
				}
			}
		}
	}

	return dst;
}

cv::Mat CvTranslator::FrostedGlass(const cv::Mat & imageSource)
{
	cv::Mat imageResult = imageSource.clone();
	cv::RNG rng;
	int randomNum;
	int Number = 5;

	for (int i = 0; i < imageSource.rows - Number; i++)
		for (int j = 0; j < imageSource.cols - Number; j++)
		{
			randomNum = rng.uniform(0, Number);
			imageResult.at<cv::Vec3b>(i, j)[0] = imageSource.at<cv::Vec3b>(i + randomNum, j + randomNum)[0];
			imageResult.at<cv::Vec3b>(i, j)[1] = imageSource.at<cv::Vec3b>(i + randomNum, j + randomNum)[1];
			imageResult.at<cv::Vec3b>(i, j)[2] = imageSource.at<cv::Vec3b>(i + randomNum, j + randomNum)[2];
		}
	return imageResult;
}


cv::Mat CvTranslator::simpleSkinSmoothing(const cv::Mat& inputImage) {
	int d = 15; double sigmaColor = 150; double sigmaSpace = 15;
	if (inputImage.empty()) {
		std::cerr << "skinsmooth: input is null" << std::endl;
		return inputImage;
	}

	cv::Mat smoothedImage;
	bilateralFilter(inputImage, smoothedImage, d, sigmaColor, sigmaSpace);

	return smoothedImage;
}
/*
cv::Mat CvTranslator::addTextWatermark(const cv::Mat& src, const std::string& text) {
	//初始化
	const cv::Point& pos = cv::Point(50, 50);
	double fontScale = 1.0;
	cv::Scalar color = cv::Scalar(0, 0, 255);  // BGR格式，默认红色
	int thickness = 2;
	int lineType = cv::LINE_AA;



	cv::Mat dst = src.clone();  // 克隆原图，避免修改原图

	// 设置字体（OpenCV支持的字体）
	int fontFace = cv::FONT_HERSHEY_SIMPLEX;  // 常用字体

	// 绘制文字水印
	cv::putText(dst, text, pos, fontFace, fontScale, color, thickness, lineType);

	return dst;
}
*/
cv::Mat CvTranslator::Whitening(const cv::Mat& src) {
	if (src.empty()) return cv::Mat();

	cv::Mat dst = src.clone();
	// 转换为浮点型以避免运算溢出（中间过程使用更高精度）
	src.convertTo(dst, CV_32F);

	// 定义美白参数（与原逻辑保持一致）
	double alpha = 1.3;    // 比例系数
	int beta = 30;         // 偏移量

	// 矩阵运算：dst = src * alpha + beta（替代双重循环）
	dst = dst * alpha + beta;

	// 转换回原类型，并自动截断超出[0,255]的像素值（替代saturate_cast）
	dst.convertTo(dst, src.type(), 1.0, 0.0);

	return dst;
}

cv::Mat CvTranslator::Whitening2(const cv::Mat &src) {
	std::vector<int> Color_list = {
	1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 31, 33, 35, 37, 39,
	41, 43, 44, 46, 48, 50, 52, 53, 55, 57, 59, 60, 62, 64, 66, 67, 69, 71, 73, 74,
	76, 78, 79, 81, 83, 84, 86, 87, 89, 91, 92, 94, 95, 97, 99, 100, 102, 103, 105,
	106, 108, 109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 124, 126, 127, 128,
	130, 131, 133, 134, 135, 137, 138, 139, 141, 142, 143, 145, 146, 147, 149, 150,
	151, 153, 154, 155, 156, 158, 159, 160, 161, 162, 164, 165, 166, 167, 168, 170,
	171, 172, 173, 174, 175, 176, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187,
	188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,
	204, 205, 205, 206, 207, 208, 209, 210, 211, 211, 212, 213, 214, 215, 215, 216,
	217, 218, 219, 219, 220, 221, 222, 222, 223, 224, 224, 225, 226, 226, 227, 228,
	228, 229, 230, 230, 231, 232, 232, 233, 233, 234, 235, 235, 236, 236, 237, 237,
	238, 238, 239, 239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 244, 245,
	245, 246, 246, 246, 247, 247, 248, 248, 248, 249, 249, 249, 250, 250, 250, 250,
	251, 251, 251, 251, 252, 252, 252, 252, 253, 253, 253, 253, 253, 254, 254, 254,
	254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 256 };
	cv::Mat dst = src.clone();
	// 转换为浮点型以避免运算溢出（中间过程使用更高精度）

	if (Color_list.size() <= 255) {
		std::cerr << "错误：Color_list长度不足，无法覆盖0~255索引" << std::endl;
		return cv::Mat();
	}

	// 遍历像素进行颜色映射（使用指针访问提升效率）
	for (int i = 0; i < dst.rows; ++i) {
		// 获取第i行的指针（CV_8UC3对应cv::Vec3b）
		cv::Vec3b* row_ptr = dst.ptr<cv::Vec3b>(i);
		for (int j = 0; j < dst.cols; ++j) {
			// 分别对B、G、R通道进行映射（注意OpenCV默认是BGR顺序）
			int b = row_ptr[j][0];  // 原B通道值（0~255）
			int g = row_ptr[j][1];  // 原G通道值
			int r = row_ptr[j][2];  // 原R通道值

			// 映射到Color_list中的值（确保索引在有效范围）
			row_ptr[j][0] = static_cast<uchar>(Color_list[b]);  // 新B通道
			row_ptr[j][1] = static_cast<uchar>(Color_list[g]);  // 新G通道
			row_ptr[j][2] = static_cast<uchar>(Color_list[r]);  // 新R通道
		}
	}

	// 转换回原类型，并自动截断超出[0,255]的像素值（替代saturate_cast）

	return dst;
}


cv::Mat CvTranslator::addTextWatermark(const cv::Mat& src, const std::string& text,
	const cv::Point& pos ,
	double fontScale,
	cv::Scalar color ,  // BGR格式，默认红色
	int thickness ,
	int lineType ) {
	cv::Mat dst = src.clone();  // 克隆原图，避免修改原图

	// 设置字体（OpenCV支持的字体）
	int fontFace = cv::FONT_HERSHEY_SIMPLEX;  // 常用字体

	// 绘制文字水印
	cv::putText(dst, text, pos, fontFace, fontScale, color, thickness, lineType);

	return dst;
}

cv::Mat CvTranslator::invertImage(cv::Mat src)
{
	// 图像取反
	cv::Mat invertedImage;
	cv::bitwise_not(src, invertedImage);
	return invertedImage;
}

// -------------------- CvTranslator C API --------------------
static bool cvtranslator_imread(const char* path, cv::Mat& out)
{
	if (!path || !*path) {
		return false;
	}
	out = cv::imread(path, cv::IMREAD_COLOR);
	return !out.empty();
}

static bool cvtranslator_imwrite(const char* path, const cv::Mat& img)
{
	if (!path || !*path || img.empty()) {
		return false;
	}
	return cv::imwrite(path, img);
}

extern "C" OPENCVFFMPEGTOOLS_API void* CvTranslator_Create()
{
	return new CvTranslator();
}

extern "C" OPENCVFFMPEGTOOLS_API void CvTranslator_Destroy(void* translator)
{
	delete static_cast<CvTranslator*>(translator);
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_GrayImage_File(void* translator, const char* input_path, const char* output_path)
{
	try {
		if (!translator) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Mat dst = static_cast<CvTranslator*>(translator)->grayImage(src);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_Invert_File(void* translator, const char* input_path, const char* output_path)
{
	try {
		if (!translator) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Mat dst = static_cast<CvTranslator*>(translator)->invertImage(src);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_FrostedGlass_File(void* translator, const char* input_path, const char* output_path)
{
	try {
		if (!translator) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Mat dst = static_cast<CvTranslator*>(translator)->FrostedGlass(src);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_SkinSmoothing_File(void* translator, const char* input_path, const char* output_path)
{
	try {
		if (!translator) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Mat dst = static_cast<CvTranslator*>(translator)->simpleSkinSmoothing(src);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_Whitening_File(void* translator, const char* input_path, const char* output_path)
{
	try {
		if (!translator) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Mat dst = static_cast<CvTranslator*>(translator)->Whitening(src);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_Whitening2_File(void* translator, const char* input_path, const char* output_path)
{
	try {
		if (!translator) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Mat dst = static_cast<CvTranslator*>(translator)->Whitening2(src);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_OilPainting_File(void* translator, const char* input_path, const char* output_path, int radius, double sigma_color)
{
	try {
		if (!translator) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Mat dst = static_cast<CvTranslator*>(translator)->applyOilPainting(src, radius, sigma_color);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_Mosaic_File(void* translator, const char* input_path, const char* output_path, int x, int y, int w, int h, int cellSize)
{
	try {
		if (!translator) return false;
		if (cellSize <= 0) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Rect region(x, y, w, h);
		cv::Mat dst = static_cast<CvTranslator*>(translator)->applyMosaic(src, region, cellSize);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_AddTextWatermark_File(void* translator, const char* input_path, const char* output_path, const char* text)
{
	try {
		if (!translator || !text) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Mat dst = static_cast<CvTranslator*>(translator)->addTextWatermark(src, std::string(text));
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

extern "C" OPENCVFFMPEGTOOLS_API bool CvTranslator_AddTextWatermarkEx_File(void* translator, const char* input_path, const char* output_path, const char* text, int x, int y, double fontScale, int b, int g, int r, int thickness)
{
	try {
		if (!translator || !text) return false;
		cv::Mat src;
		if (!cvtranslator_imread(input_path, src)) return false;
		cv::Point pos(x, y);
		cv::Scalar color(b, g, r);
		cv::Mat dst = static_cast<CvTranslator*>(translator)->addTextWatermark(src, std::string(text), pos, fontScale, color, thickness, cv::LINE_AA);
		return cvtranslator_imwrite(output_path, dst);
	}
	catch (...) {
		return false;
	}
}

