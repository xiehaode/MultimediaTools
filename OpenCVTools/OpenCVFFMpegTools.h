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
#include <stdint.h>
// C 语言下的结构体前向声明（兼容C编译）
typedef struct PlayerConfig PlayerConfig;
typedef struct DecodedFrame DecodedFrame;



enum func {
	grayImage,
	customOilPaintApprox,
	applyOilPainting,
	applyMosaic,
	FrostedGlass,
	simpleSkinSmoothing,
	Whitening,
	Whitening2,
	addTextWatermark,
	invertImage
};






// 
// C++/OpenCV 相关能力请包含 `COpenCVTools.h`
#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif




// ---- AvWorker C API（基于文件输入输出，外部无需 OpenCV 头文件）----



// ----  C API（基于文件输入输出，外部无需 OpenCV 头文件）----
extern OPENCVFFMPEGTOOLS_API int nOpenCVTools;
OPENCVFFMPEGTOOLS_API int fnOpenCVTools(void);
// ---- AvWorker C API（基于文件输入输出，外部无需 OpenCV 头文件）----
OPENCVFFMPEGTOOLS_API void* AvWorker_Create();
OPENCVFFMPEGTOOLS_API void AvWorker_Destroy(void* worker);
OPENCVFFMPEGTOOLS_API bool AvWorker_GetVideoFirstFrame(void* worker, const char* input_url, const char* output_bmp, bool is_rtsp);
OPENCVFFMPEGTOOLS_API bool AvWorker_SpliceAV(void* worker, const char* input_url1, const char* input_url2, const char* output_url, bool is_rtsp);
OPENCVFFMPEGTOOLS_API bool AvWorker_resize_video(void* worker, const char* input_url, const char* output_url, int dst_width, int dst_height);
OPENCVFFMPEGTOOLS_API bool AvWorker_GetVideoFirstFrame(void* worker, const char* input_url, const char* output_bmp, bool is_rtsp);
OPENCVFFMPEGTOOLS_API bool AvWorker_split_video(
	void* worker,
	const char* input_url,
	const char* output_url,
	double start_seconds,
	double duration_seconds);
OPENCVFFMPEGTOOLS_API double AvWorker_getDuration(void* worker, const char* input_url);

// ---- CvTranslator C API（基于文件输入输出，外部无需 OpenCV 头文件）----
OPENCVFFMPEGTOOLS_API void* CvTranslator_Create();
OPENCVFFMPEGTOOLS_API void CvTranslator_Destroy(void* translator);

OPENCVFFMPEGTOOLS_API bool CvTranslator_GrayImage_File(void* translator, const char* input_path, const char* output_path);
OPENCVFFMPEGTOOLS_API bool CvTranslator_Invert_File(void* translator, const char* input_path, const char* output_path);
OPENCVFFMPEGTOOLS_API bool CvTranslator_FrostedGlass_File(void* translator, const char* input_path, const char* output_path);
OPENCVFFMPEGTOOLS_API bool CvTranslator_SkinSmoothing_File(void* translator, const char* input_path, const char* output_path);
OPENCVFFMPEGTOOLS_API bool CvTranslator_Whitening_File(void* translator, const char* input_path, const char* output_path);
OPENCVFFMPEGTOOLS_API bool CvTranslator_Whitening2_File(void* translator, const char* input_path, const char* output_path);
OPENCVFFMPEGTOOLS_API bool CvTranslator_OilPainting_File(void* translator, const char* input_path, const char* output_path, int radius, double sigma_color);
OPENCVFFMPEGTOOLS_API bool CvTranslator_Mosaic_File(void* translator, const char* input_path, const char* output_path, int x, int y, int w, int h, int cellSize);
OPENCVFFMPEGTOOLS_API bool CvTranslator_AddTextWatermark_File(void* translator, const char* input_path, const char* output_path, const char* text);
OPENCVFFMPEGTOOLS_API bool CvTranslator_AddTextWatermarkEx_File(void* translator, const char* input_path, const char* output_path, const char* text, int x, int y, double fontScale, int b, int g, int r, int thickness);

// ---- FFmpegEncoder C API ----
OPENCVFFMPEGTOOLS_API void* Encoder_Create();
OPENCVFFMPEGTOOLS_API void Encoder_Destroy(void* encoder);
OPENCVFFMPEGTOOLS_API int Encoder_VideoMuxerCreate(void* encoder, const char* output_path, int width, int height, int fps);
OPENCVFFMPEGTOOLS_API int Encoder_VideoMuxerWriteFrame(void* encoder, void* frame, int frame_idx);
OPENCVFFMPEGTOOLS_API int Encoder_VideoMuxerFlush(void* encoder);

// ---- FFmpegDecoder C API ----
OPENCVFFMPEGTOOLS_API void* Decoder_Create();
OPENCVFFMPEGTOOLS_API void Decoder_Destroy(void* decoder);
OPENCVFFMPEGTOOLS_API int Decoder_FFPlayerOpen(void* decoder, const char* input_path, int is_device);
OPENCVFFMPEGTOOLS_API void Decoder_FFPlayerReadFrame(void* decoder);
OPENCVFFMPEGTOOLS_API void Decoder_FFPlayerClose(void* decoder);
OPENCVFFMPEGTOOLS_API int64_t Decoder_GetDuration(void* decoder);
OPENCVFFMPEGTOOLS_API int64_t Decoder_GetCurrentTime(void* decoder);

#ifdef __cplusplus
} // extern "C"
#endif
