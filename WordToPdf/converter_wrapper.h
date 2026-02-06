/*
 * C头文件 - 用于Qt集成
 */

#ifndef CONVERTER_WRAPPER_H
#define CONVERTER_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// 基本转换函数
int convert_file(const char* input_path, const char* output_path);

// 批量转换函数
int convert_files_batch(const char* const* input_paths, int file_count, 
                       const char* output_dir, const char* output_format);

// 文件支持检查
int check_file_support(const char* file_path);

// 获取支持格式
const char* get_supported_formats();

// 获取文件类型
const char* get_file_type(const char* file_path);

// 错误处理函数
char* get_last_error();
void clear_error();

#ifdef __cplusplus
}
#endif

#endif // CONVERTER_WRAPPER_H