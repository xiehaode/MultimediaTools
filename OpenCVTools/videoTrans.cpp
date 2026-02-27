#include "pch.h"
#include "videoTrans.h"
#include "CvTranslator.h"
#include "COpenCVTools.h"
#include <string>

videoTrans::videoTrans()
	: decoder(nullptr), encoder(nullptr)
	, m_initialized(false)
	, m_width(0)
	, m_height(0)
	, m_fps(30) // 默认帧率
	, m_duration(0)
{
	decoder = new FFmpegDecoder();
	encoder = new FFmpegEncoder();
}

videoTrans::~videoTrans()
{
	cleanup(); // 使用统一的清理函数
}

int videoTrans::initialize(const std::string& inputPath, const std::string& outputPath)
{
	// 参数验证
	if (inputPath.empty() || outputPath.empty()) {
		return -1; // 无效参数
	}
	
	// 如果已经初始化，先清理
	if (m_initialized) {
		cleanup();
	}
	
	// 打开解码器
	if (decoder->ffplayer_open(inputPath, false) != 0) {
		return -2; // 打开输入文件失败
	}
	
	//获取输入视频的实际属性
	m_width = decoder->getWidth();
	m_height = decoder->getHeight();
	m_fps = decoder->getFPS();
	m_duration = decoder->getDuration();
	
	printf("输入视频属性: %dx%d, %dfps, 时长: %lldms\n", m_width, m_height, m_fps, (long long)m_duration);
	
	// 验证获取到的视频属性
	if (m_width <= 0 || m_height <= 0) {
		decoder->ffplayer_close();
		return -3; // 无法获取视频尺寸
	}
	
	// 初始化编码器 - 使用输入视频的实际属性
	if (encoder->video_muxer_create(outputPath.c_str(), m_width, m_height, m_fps) != 0) {
		decoder->ffplayer_close();
		return -4; // 创建编码器失败
	}
	
	// 保存路径和状态
	m_inputPath = inputPath;
	m_outputPath = outputPath;
	m_initialized = true;
	
	printf("videoTrans初始化成功: %s -> %s\n", inputPath.c_str(), outputPath.c_str());
	return 0; // 成功
}

int videoTrans::getWidth() const
{
	return m_width;
}

int videoTrans::getHeight() const
{
	return m_height;
}

int videoTrans::getFPS() const
{
	return m_fps;
}

int64_t videoTrans::getDuration() const
{
	return m_duration;
}

int videoTrans::reset()
{
	if (!m_initialized) {
		return -1; // 未初始化
	}
	
	// 关闭当前解码器
	decoder->ffplayer_close();
	
	// 重新打开
	if (decoder->ffplayer_open(m_inputPath, false) != 0) {
		return -2; // 重新打开失败
	}
	
	return 0; // 成功
}

void videoTrans::cleanup()
{
	if (decoder) {
		decoder->ffplayer_close();
	}
	if (encoder) {
		encoder->video_muxer_destroy();
	}
	
	m_initialized = false;
	m_inputPath.clear();
	m_outputPath.clear();
	m_width = 0;
	m_height = 0;
	m_fps = 30;
	m_duration = 0;
}

int videoTrans::process(func fun, param mParem)
{
	// 检查初始化状态
	if (!m_initialized) {
		return -1; // 未初始化
	}
	
	// 创建工具实例
	CvTranslator translator;
	COpenCVTools cvTools;
	
	// 逐帧处理
	int frameIdx = 0;
	const int maxFrames = 100000; // 限制最大帧数，避免无限循环
	
	while (frameIdx < maxFrames) {
		int readRet = decoder->read_frame_for_trans();
		if (readRet == 0) {
			// EOF
			break;
		}
		if (readRet < 0) {
			printf("读取帧失败，错误代码: %d\n", readRet);
			break;
		}
		
		// 获取当前解码的AVFrame
		AVFrame* currentFrame = decoder->getCurrentAVFrame();
		if (!currentFrame) {
			break;
		}
		
		// 检查帧是否有效
		if (currentFrame->width > 0 && currentFrame->height > 0) {
			
			// 使用COpenCVTools将AVFrame转换为cv::Mat
			cv::Mat mat = cvTools.AVFrameToCVMat(currentFrame);
			if (mat.empty()) {
				frameIdx++;
				continue; // 转换失败，跳过此帧
			}
			
			// 根据func参数应用不同的图像处理
			cv::Mat processedFrame;
			switch (fun) {
				case grayImage:
					processedFrame = translator.grayImage(mat);
					break;
					
				case customOilPaintApprox:
					processedFrame = translator.customOilPaintApprox(mat, mParem.iparam1, mParem.dparam1);
					break;
					
				case applyOilPainting:
					processedFrame = translator.applyOilPainting(mat, mParem.iparam1, mParem.dparam1);
					break;
					
				case applyMosaic:
					{
						cv::Rect mosaicRegion(mParem.iparam1, mParem.iparam2, mParem.iparam3, mParem.iparam4); // 示例区域
						processedFrame = translator.applyMosaic(mat, mosaicRegion, mParem.iparam5);
					}
					break;
					
				case FrostedGlass:
					processedFrame = translator.FrostedGlass(mat);
					break;
					
				case simpleSkinSmoothing:
					processedFrame = translator.simpleSkinSmoothing(mat);
					break;
					
				case Whitening:
					processedFrame = translator.Whitening(mat);
					break;
					
				case Whitening2:
					processedFrame = translator.Whitening2(mat);
					break;
					
				case addTextWatermark:
					processedFrame = translator.addTextWatermark(mat, mParem.arr, cv::Point(mParem.iparam1, mParem.iparam2));
					break;
					
				case invertImage:
					processedFrame = translator.invertImage(mat);
					break;
					
				default:
					processedFrame = mat.clone(); // 不处理，直接复制
					break;
			}
			
			// 使用COpenCVTools将处理后的cv::Mat转换回AVFrame
			AVFrame* outputFrame = cvTools.CVMatToAVFrame(processedFrame);
			if (outputFrame) {
				// 设置时间戳
				outputFrame->pts = frameIdx;
				
				// 写入编码器
				int ret = encoder->video_muxer_write_frame(outputFrame, frameIdx);
				
				// 释放AVFrame
				av_frame_free(&outputFrame);
				
				if (ret != 0) {
					printf("写入帧失败，帧索引: %d, 错误代码: %d\n", frameIdx, ret);
					break; // 写入失败，退出循环
				}
			} else {
				printf("转换AVFrame失败，帧索引: %d - 跳过该帧\n", frameIdx);
				frameIdx++;
				continue; // 跳过转换失败的帧，继续处理下一帧
			}
			
			frameIdx++;
		}
	}
	
	printf("总共处理了 %d 帧\n", frameIdx);
	
	// 完成编码
	encoder->video_muxer_flush();
	
	return 0; // 成功
}

int videoTrans::trans(const std::string& inputPath, const std::string& outputPath, func fun , param m)
{
	// 使用新的初始化和process方法
	int result = initialize(inputPath, outputPath);
	if (result != 0) {
		return result; // 初始化失败
	}
	
	return process(fun,m); // 处理视频
}

// 为了兼容原来的接口，提供一个重载版本
int videoTrans::trans(func fun,param m)
{
	// 使用默认的输入输出路径
	return this->trans("input.mp4", "output.mp4", fun,m);
}

// C接口实现
extern "C" OPENCVFFMPEGTOOLS_API void* VideoTrans_Create()
{
	return new videoTrans();
}

extern "C" OPENCVFFMPEGTOOLS_API void VideoTrans_Destroy(void* trans)
{
	if (trans) {
		delete static_cast<videoTrans*>(trans);
	}
}

extern "C" OPENCVFFMPEGTOOLS_API int VideoTrans_Initialize(void* trans, const char* input_path, const char* output_path)
{
	if (!trans || !input_path || !output_path) return -1;
	return static_cast<videoTrans*>(trans)->initialize(std::string(input_path), std::string(output_path));
}

extern "C" OPENCVFFMPEGTOOLS_API int VideoTrans_GetWidth(void* trans)
{
	if (!trans) return 0;
	return static_cast<videoTrans*>(trans)->getWidth();
}

extern "C" OPENCVFFMPEGTOOLS_API int VideoTrans_GetHeight(void* trans)
{
	if (!trans) return 0;
	return static_cast<videoTrans*>(trans)->getHeight();
}

extern "C" OPENCVFFMPEGTOOLS_API int VideoTrans_GetFPS(void* trans)
{
	if (!trans) return 30;
	return static_cast<videoTrans*>(trans)->getFPS();
}

extern "C" OPENCVFFMPEGTOOLS_API int64_t VideoTrans_GetDuration(void* trans)
{
	if (!trans) return 0;
	return static_cast<videoTrans*>(trans)->getDuration();
}

extern "C" OPENCVFFMPEGTOOLS_API int VideoTrans_Process(void* trans, int effect_type, param m)
{
	if (!trans) return -1;
	func effect = static_cast<func>(effect_type);
	return static_cast<videoTrans*>(trans)->process(effect,m);
}

extern "C" OPENCVFFMPEGTOOLS_API int VideoTrans_Reset(void* trans)
{
	if (!trans) return -1;
	return static_cast<videoTrans*>(trans)->reset();
}

extern "C" OPENCVFFMPEGTOOLS_API void VideoTrans_Cleanup(void* trans)
{
	if (!trans) return;
	static_cast<videoTrans*>(trans)->cleanup();
}

/*

int main() {
	// 1. 定义输入/输出路径
	const char* inputVideo = "input.mp4";
	const char* outputVideo = "output_mosaic.mp4";

	// 2. 创建videoTrans句柄（C接口：VideoTrans_Create）
	void* pTrans = VideoTrans_Create();
	if (!pTrans) {
		printf("创建VideoTrans句柄失败！\n");
		return -1;
	}

	// 3. 初始化（C接口：VideoTrans_Initialize）
	int initRet = VideoTrans_Initialize(pTrans, inputVideo, outputVideo);
	if (initRet != 0) {
		printf("初始化失败，错误码：%d\n", initRet);
		VideoTrans_Destroy(pTrans);
		return initRet;
	}

	// 4. 打印视频属性（C接口获取宽/高/帧率/时长）
	int width = VideoTrans_GetWidth(pTrans);
	int height = VideoTrans_GetHeight(pTrans);
	int fps = VideoTrans_GetFPS(pTrans);
	int64_t duration = VideoTrans_GetDuration(pTrans);
	printf("视频属性：%dx%d | %dfps | 时长：%llds\n",
		   width, height, fps, duration / 1000);

	// 5. 执行视频处理（C接口：VideoTrans_Process，传入特效类型int值）
	// 特效类型：applyMosaic（马赛克），直接传枚举对应的int值
	int processRet = VideoTrans_Process(pTrans, applyMosaic);
	if (processRet != 0) {
		printf("视频处理失败，错误码：%d\n", processRet);
		VideoTrans_Destroy(pTrans);
		return processRet;
	}

	// 6. 重置解码器（可选，重新处理同一路径视频时使用）
	// int resetRet = VideoTrans_Reset(pTrans);
	// if (resetRet != 0) { printf("重置失败：%d\n", resetRet); }

	// 7. 手动清理资源（可选，Destroy会自动调用）
	// VideoTrans_Cleanup(pTrans);

	// 8. 销毁句柄（释放所有资源，必须调用）
	VideoTrans_Destroy(pTrans);
	pTrans = nullptr;

	printf("视频处理完成！输出路径：%s\n", outputVideo);
	return 0;
}

*/





