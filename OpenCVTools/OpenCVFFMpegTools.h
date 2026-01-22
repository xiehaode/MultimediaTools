#pragma once

// DLL export/import
// 注意：本工程 vcxproj 里使用的是 OPENCVTOOLS_EXPORTS 作为导出开关
#ifdef OPENCVTOOLS_EXPORTS
#define OPENCVFFMPEGTOOLS_API __declspec(dllexport)
#else
#define OPENCVFFMPEGTOOLS_API __declspec(dllimport)
#endif

#include <cstddef>
#include <cstdint>

// ---- C API（可用于 LoadLibrary/GetProcAddress，外部无需链接任何 .lib）----
#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

OPENCVFFMPEGTOOLS_API void* AvWorker_Create();
OPENCVFFMPEGTOOLS_API void AvWorker_Destroy(void* worker);
OPENCVFFMPEGTOOLS_API bool AvWorker_GetVideoFirstFrame(void* worker, const char* input_url, const char* output_bmp, bool is_rtsp);

#ifdef __cplusplus
} // extern "C"
#endif

// ---- 可选的 C++ API（暴露 OpenCV 类型）----
// 默认不暴露，避免外部工程必须配置 OpenCV include。
// 如果你希望使用 cv::Mat / AVFrame 相关接口，请在包含此头前定义：
//   OPENCVFFMPEGTOOLS_EXPOSE_OPENCV_TYPES
#ifdef __cplusplus

struct AVFrame;

extern OPENCVFFMPEGTOOLS_API int nOpenCVTools;
OPENCVFFMPEGTOOLS_API int fnOpenCVTools(void);

#ifdef OPENCVFFMPEGTOOLS_EXPOSE_OPENCV_TYPES
#include <opencv2/core/mat.hpp>

// 此类是从 dll 导出的（注意：该类的头会暴露 OpenCV 类型）
class OPENCVFFMPEGTOOLS_API COpenCVTools {
public:
	COpenCVTools();
	void test();
	cv::Mat AVFrameToCVMat(AVFrame* yuv420Frame);
	AVFrame* CVMatToAVFrame(cv::Mat& inMat);
};
#endif // OPENCVFFMPEGTOOLS_EXPOSE_OPENCV_TYPES

#endif // __cplusplus
