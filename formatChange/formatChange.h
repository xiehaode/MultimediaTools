// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 FORMATCHANGE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// FORMATCHANGE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#include <string>

#ifdef FORMATCHANGE_EXPORTS
#define FORMATCHANGE_API __declspec(dllexport)
#else
#define FORMATCHANGE_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 前向声明 AVConfig 结构体
//typedef struct AVConfig AVConfig;

struct AVConfig {
	// 通用参数
	int bit_rate = 0;          // 码率（0表示使用默认值）
	int width = 0;             // 视频宽度（0表示使用源尺寸）
	int height = 0;            // 视频高度（0表示使用源尺寸）
	int frame_rate = 25;       // 帧率（默认25fps）
	int sample_rate = 44100;   // 音频采样率（默认44100Hz）
	int channels = 2;          // 音频声道数（默认立体声）

	// GIF专用参数
	int gif_delay = 10;        // GIF帧延迟（单位：1/100秒，默认10即100ms）
	int gif_loop = 0;          // GIF循环次数（0表示无限循环）

	// 时间范围参数
	double start_time = 0.0;   // 处理起始时间（秒）
	double duration = 0.0;     // 处理时长（秒，0表示处理全部）

	// 图片序列参数
	std::string img_pattern;   // 图片序列路径模板（如 "frame_%04d.jpg"）
	int img_start_idx = 0;     // 图片序列起始索引
	int img_end_idx = -1;      // 图片序列结束索引（-1表示自动检测）
};



FORMATCHANGE_API void* AVProcessor_Create();
FORMATCHANGE_API void AVProcessor_Destroy(void* processor);
FORMATCHANGE_API int AVProcessor_Remux(void* processor, const char* input_path, const char* output_path);
FORMATCHANGE_API int AVProcessor_Transcode(void* processor, const char* input_path, const char* output_path, const AVConfig* config);
FORMATCHANGE_API int AVProcessor_Mp4ToGif(void* processor, const char* mp4_path, const char* gif_path, const AVConfig* config);
FORMATCHANGE_API int AVProcessor_ImgSeqToMp4(void* processor, const char* output_path, const AVConfig* config);

#ifdef __cplusplus
}
#endif
