#include "pch.h"
#include "../OpenCVTools/OpenCVFFMpegTools.h"

#include <string>

#include <Windows.h>

#include <cstdio>
#include <iostream>

int main() {
	// 1. 定义输入/输出路径
	// 使用绝对路径，避免调试时工作目录变化导致找不到文件
	const char* inputVideo = "D:/vsPro/MultiMediaTool/bin/1.mp4";
	const char* outputVideo = "D:/vsPro/MultiMediaTool/bin/output_addTextWatermark.mp4";


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
	int processRet = VideoTrans_Process(pTrans, addTextWatermark);
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