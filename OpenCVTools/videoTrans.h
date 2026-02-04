#pragma once
#include "COpenCVTools.h"
#include "CvTranslator.h"
#include "FFmpegDecoder.h"
#include "FFmpegEncoder.h"
class videoTrans
{
public:
	videoTrans();
	~videoTrans();
	
	// 初始化函数 - 打开解码器并设置参数
	int initialize(const std::string& inputPath, const std::string& outputPath);
	
	// 获取视频属性信息
	int getWidth() const;
	int getHeight() const;
	int getFPS() const;
	int64_t getDuration() const;
	
	// 处理函数 - 使用已初始化的解码器
	int process(func fun);
	
	// 重置解码器到开始位置
	int reset();
	
	// 清理资源
	void cleanup();
	
	// 兼容性接口
	int trans(func fun); // 兼容性接口，使用默认路径
	int trans(const std::string& inputPath, const std::string& outputPath, func fun); // 完整接口

private:
	FFmpegDecoder* decoder;
	FFmpegEncoder* encoder;
	
	// 初始化状态跟踪
	bool m_initialized;
	std::string m_inputPath;
	std::string m_outputPath;
	
	// 视频属性缓存
	int m_width;
	int m_height;
	int m_fps;
	int64_t m_duration;
};

