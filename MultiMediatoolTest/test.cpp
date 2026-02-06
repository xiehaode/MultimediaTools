#include "pch.h"
#include "../OpenCVTools/OpenCVFFMpegTools.h"
#include "../formatChange/formatChange.h"
#include <string>

#include <Windows.h>

#include <cstdio>
#include <iostream>

int main() {
	/*
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
	*/

	printf("===== 测试转封装功能 =====\n");
	// 创建AVProcessor实例
	void* processor = AVProcessor_Create();
	if (!processor) {
		printf("创建AVProcessor实例失败！\n");
		return -1;
	}

	// 调用转封装：输入MP4，输出FLV
	const char* input_path = "D:/vsPro/MultiMediaTool/bin/1.mp4";
	const char* output_path = "test_output.flv";
	int ret = AVProcessor_Remux(processor, input_path, output_path);
	if (ret == 0) {
		printf("转封装成功：%s → %s\n", input_path, output_path);
	}
	else {
		printf("转封装失败！错误码：%d\n", ret);
	}

	// ======================================
	// 2. 进阶示例：调用转码功能（配置参数）
	// ======================================
	printf("\n===== 测试转码功能框架 =====\n");
	AVConfig config = { 0 };
	// 设置转码配置
	config.width = 1280;          // 输出宽度
	config.height = 720;         // 输出高度
	//config.bitrate = 2000000;    // 2Mbps比特率
	//config.fps = 30;             // 30帧/秒
	//strcpy(config.codec_name, "h264"); // H.264编码器

	// 调用转码：输入MP4，输出新MP4（修改分辨率/编码）
	const char* transcode_input = "2.mp4";
	const char* transcode_output = "test_transcode.mp4";
	ret = AVProcessor_Transcode(processor, transcode_input, transcode_output, &config);
	if (ret == 0) {
		printf("转码框架调用成功（需补充完整编码逻辑）\n");
	}
	else {
		printf("转码调用失败！错误码：%d\n", ret);
	}

	// ======================================
	// 3. 测试MP4转GIF（框架）
	// ======================================
	printf("\n===== 测试MP4转GIF功能框架 =====\n");
	AVConfig gif_config = { 0 };
	gif_config.width = 640;
	gif_config.height = 360;
	//gif_config.fps = 10; // GIF帧率

	const char* gif_input = "D:/vsPro/MultiMediaTool/bin/1.mp4";
	const char* gif_output = "test_output.gif";
	ret = AVProcessor_Mp4ToGif(processor, gif_input, gif_output, &gif_config);
	if (ret == 0) {
		printf("MP4转GIF框架调用成功（需补充完整逻辑）\n");
	}
	else {
		printf("MP4转GIF调用失败！错误码：%d\n", ret);
	}

	// ======================================
	// 4. 测试图片序列转MP4（框架）
	// ======================================
	printf("\n===== 测试图片序列转MP4功能框架 =====\n");
	AVConfig img_config = { 0 };
	img_config.width = 1920;
	img_config.height = 1080;
	//img_config.fps = 25;
	//strcpy(img_config.codec_name, "h264");

	const char* img_output = "D:/vsPro/MultiMediaTool/bin/1.mp4";
	ret = AVProcessor_ImgSeqToMp4(processor, img_output, &img_config);
	if (ret == 0) {
		printf("图片序列转MP4框架调用成功（需补充完整逻辑）\n");
	}
	else {
		printf("图片序列转MP4调用失败！错误码：%d\n", ret);
	}

	// ======================================
	// 5. 释放资源
	// ======================================
	AVProcessor_Destroy(processor);
	printf("\n所有测试完成，资源已释放！\n");
	return 0;
}