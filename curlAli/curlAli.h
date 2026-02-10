// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 CURLALI_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// CURLALI_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef CURLALI_EXPORTS
#define CURLALI_API __declspec(dllexport)
#else
#define CURLALI_API __declspec(dllimport)
#endif


extern CURLALI_API int ncurlAli;


#ifdef __cplusplus
extern "C" {
#endif

// ---- DashScopeChat C API ----
CURLALI_API void* DashScopeChat_Create(const char* apiKey);
CURLALI_API void DashScopeChat_Destroy(void* chat);

/**
 * @brief 发送聊天消息
 * @param chat 实例指针
 * @param message 用户消息
 * @return 响应内容
 */
CURLALI_API const char* DashScopeChat_Chat(void* chat, const char* message);

#ifdef __cplusplus
}
#endif

