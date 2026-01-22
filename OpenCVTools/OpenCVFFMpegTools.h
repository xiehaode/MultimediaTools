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
// C++/OpenCV 相关能力请包含 `COpenCVTools.h`
#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

extern OPENCVFFMPEGTOOLS_API int nOpenCVTools;
OPENCVFFMPEGTOOLS_API int fnOpenCVTools(void);

OPENCVFFMPEGTOOLS_API void* AvWorker_Create();
OPENCVFFMPEGTOOLS_API void AvWorker_Destroy(void* worker);
OPENCVFFMPEGTOOLS_API bool AvWorker_GetVideoFirstFrame(void* worker, const char* input_url, const char* output_bmp, bool is_rtsp);

#ifdef __cplusplus
} // extern "C"
#endif
