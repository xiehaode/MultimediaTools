// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 VIDEOCAPTURE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// VIDEOCAPTURE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef VIDEOCAPTURE_EXPORTS
#define VIDEOCAPTURE_API __declspec(dllexport)
#else
#define VIDEOCAPTURE_API __declspec(dllimport)
#endif

// 录制类型枚举（对外可见）
typedef enum {
	RECORD_TYPE_GDI_CAMERA,
	RECORD_TYPE_DXGI_SCREEN
} RecordType;

// 上下文句柄（封装内部类实例，对外仅见void*）
typedef void* RecordHandle;

#ifdef __cplusplus
extern "C" {
#endif

	// 1. 创建上下文（封装RecordContext+GDIRecord/DXGIRecord）
	VIDEOCAPTURE_API RecordHandle Record_Create();

	// 2. 设置录制策略（选择GDIRecord/DXGIRecord）
	VIDEOCAPTURE_API int Record_SetStrategy(RecordHandle handle, RecordType type);

	// 3. 初始化（创建圆形窗口+初始化录制环境）
	VIDEOCAPTURE_API int Record_Init(RecordHandle handle, const wchar_t* windowTitle, int width, int height);

	// 4. 启动录制
	VIDEOCAPTURE_API int Record_Start(RecordHandle handle, const char* saveFileName);

	// 5. 停止录制
	VIDEOCAPTURE_API int Record_Stop(RecordHandle handle);

	// 6. 销毁上下文（释放所有内部类内存）
	VIDEOCAPTURE_API void Record_Destroy(RecordHandle handle);
