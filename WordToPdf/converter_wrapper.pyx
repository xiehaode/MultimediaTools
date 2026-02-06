"""
Cython包装器 - 用于将Python转换器打包为DLL供Qt使用
"""

from libc.stdlib cimport malloc, free
from libc.string cimport strcpy, strlen
import os
import sys

# 添加当前目录到Python路径
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from universal_converter import UniversalConverter

# 全局转换器实例
cdef UniversalConverter _converter = UniversalConverter()

# 错误缓冲区
cdef char* _error_buffer = NULL

cdef void set_error(const char* error_msg):
    """设置错误信息"""
    global _error_buffer
    if _error_buffer:
        free(_error_buffer)
    
    cdef size_t len = strlen(error_msg) + 1
    _error_buffer = <char*>malloc(len)
    if _error_buffer:
        strcpy(_error_buffer, error_msg)

# 导出函数声明
cdef extern from "converter_wrapper.h":
    char* get_last_error()
    void clear_error()


# 转换函数
cpdef int convert_file(const char* input_path, const char* output_path) except -1:
    """
    转换单个文件
    
    Args:
        input_path: 输入文件路径
        output_path: 输出文件路径
    
    Returns:
        1: 成功
        0: 失败
    """
    try:
        input_file = input_path.decode('utf-8')
        output_file = output_path.decode('utf-8')
        
        result = _converter.convert(input_file, output_file)
        if result:
            clear_error()
            return 1
        else:
            set_error(b"File conversion failed")
            return 0
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return 0

cpdef int convert_files_batch(const char* const* input_paths, int file_count, 
                            const char* output_dir, const char* output_format) except -1:
    """
    批量转换文件
    
    Args:
        input_paths: 输入文件路径数组
        file_count: 文件数量
        output_dir: 输出目录
        output_format: 输出格式
    
    Returns:
        1: 成功
        0: 失败
    """
    try:
        input_files = []
        for i in range(file_count):
            file_path = input_paths[i].decode('utf-8')
            input_files.append(file_path)
        
        output_directory = output_dir.decode('utf-8')
        format_name = output_format.decode('utf-8')
        
        results = _converter.convert_batch(input_files, output_directory, format_name)
        success_count = sum(1 for success in results.values() if success)
        
        if success_count > 0:
            clear_error()
            return 1
        else:
            set_error(b"All file conversions failed")
            return 0
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return 0

cpdef int check_file_support(const char* file_path) except -1:
    """
    检查文件是否支持
    
    Args:
        file_path: 文件路径
    
    Returns:
        1: 支持
        0: 不支持
    """
    try:
        file_path_str = file_path.decode('utf-8')
        result = _converter.validate_file(file_path_str)
        return 1 if result else 0
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return 0

cpdef const char* get_supported_formats():
    """
    获取支持的格式列表
    
    Returns:
        JSON格式的支持格式字符串
    """
    cdef char* result
    try:
        conversions = _converter.get_supported_conversions()
        import json
        formats_json = json.dumps(conversions, ensure_ascii=False)
        
        # 转换为C字符串
        formats_bytes = formats_json.encode('utf-8')
        result = <char*>malloc(len(formats_bytes) + 1)
        if result:
            strcpy(result, formats_bytes)
        
        return result
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return b"{}"

cpdef const char* get_file_type(const char* file_path):
    """
    获取文件类型
    
    Args:
        file_path: 文件路径
    
    Returns:
        文件类型字符串
    """
    cdef char* result
    try:
        file_path_str = file_path.decode('utf-8')
        file_type = _converter.get_file_type(file_path_str)
        if file_type:
            type_bytes = file_type.encode('utf-8')
            result = <char*>malloc(len(type_bytes) + 1)
            if result:
                strcpy(result, type_bytes)
            return result
        else:
            return b"unknown"
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return b"error"