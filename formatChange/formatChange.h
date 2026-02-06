// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 FORMATCHANGE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// FORMATCHANGE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef FORMATCHANGE_EXPORTS
#define FORMATCHANGE_API __declspec(dllexport)
#else
#define FORMATCHANGE_API __declspec(dllimport)
#endif


FORMATCHANGE_API void* AVProcessor_Create();
FORMATCHANGE_API void AVProcessor_Destroy(void* processor);
FORMATCHANGE_API int AVProcessor_Remux(void* processor, const char* input_path, const char* output_path);
FORMATCHANGE_API int AVProcessor_Transcode(void* processor, const char* input_path, const char* output_path, const AVConfig* config);
FORMATCHANGE_API int AVProcessor_Mp4ToGif(void* processor, const char* mp4_path, const char* gif_path, const AVConfig* config);
FORMATCHANGE_API int AVProcessor_ImgSeqToMp4(void* processor, const char* output_path, const AVConfig* config);
