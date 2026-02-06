// WordToPdfNative.h: WordToPdf Native DLL 的公共头文件
// 提供了用于 Word 和 PDF 格式转换的 C 接口

#ifndef WORDTOPDFNATIVE_H
#define WORDTOPDFNATIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WORDTOPDFNATIVE_EXPORTS
#define WORDTOPDFNATIVE_API __declspec(dllexport)
#else
#define WORDTOPDFNATIVE_API __declspec(dllimport)
#endif

/**
 * @brief 将 Word 文档转换为 PDF
 * @param input_path 输入的 Word 文档路径 (.docx)
 * @param output_path 输出的 PDF 文件路径
 * @return 0 表示成功，非 0 表示失败
 */
WORDTOPDFNATIVE_API int ConvertWordToPdf(const char* input_path, const char* output_path);

/**
 * @brief 将 PDF 文档转换为 Word
 * @param input_path 输入的 PDF 文件路径
 * @param output_path 输出的 Word 文档路径 (.docx)
 * @return 0 表示成功，非 0 表示失败
 */
WORDTOPDFNATIVE_API int ConvertPdfToWord(const char* input_path, const char* output_path);

/**
 * @brief 检查是否安装了 Microsoft Word
 * @return 1 表示已安装，0 表示未安装
 */
WORDTOPDFNATIVE_API int IsWordInstalled();

/**
 * @brief 获取最后一次操作的错误信息
 * @return 错误信息字符串，如果没有错误则返回空字符串
 */
WORDTOPDFNATIVE_API const char* WordToPdf_GetLastError();

/**
 * @brief 清除错误信息
 */
WORDTOPDFNATIVE_API void ClearLastError();

#ifdef __cplusplus
}
#endif

#endif // WORDTOPDFNATIVE_H