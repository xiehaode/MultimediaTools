"""
简化的转换器包装器 - 兼容较旧的Cython版本
"""

import os
import sys

# 添加当前目录到路径
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# 导入Python转换器
try:
    from WordToPdf import convert_docx_to_pdf, convert_pdf_to_docx
except ImportError:
    try:
        from universal_converter import UniversalConverter
        _converter = UniversalConverter()
    except ImportError:
        print("警告: 无法导入转换器模块")

# 错误缓冲区
cdef char* g_error_buffer = NULL

cdef void set_error(bytes error_msg):
    """设置错误信息"""
    global g_error_buffer
    if g_error_buffer:
        free(g_error_buffer)
    
    if error_msg:
        g_error_buffer = <char*>malloc(len(error_msg) + 1)
        if g_error_buffer:
            strcpy(g_error_buffer, error_msg)

cdef void clear_error():
    """清除错误信息"""
    global g_error_buffer
    if g_error_buffer:
        free(g_error_buffer)
        g_error_buffer = NULL

# 导出的C函数
cdef public int convert_file(const char* input_path, const char* output_path):
    """
    基本转换函数
    """
    clear_error()
    
    try:
        if not input_path or not output_path:
            set_error(b"Invalid input parameters")
            return 0
        
        input_str = input_path.decode('utf-8')
        output_str = output_path.decode('utf-8')
        
        # 根据文件扩展名确定转换类型
        if input_str.lower().endswith('.docx') and output_str.lower().endswith('.pdf'):
            convert_docx_to_pdf(input_str, output_str)
            return 1
        elif input_str.lower().endswith('.pdf') and output_str.lower().endswith('.docx'):
            convert_pdf_to_docx(input_str, output_str)
            return 1
        else:
            set_error(b"Unsupported file format conversion")
            return 0
            
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return 0

cdef public int check_file_support(const char* file_path):
    """
    检查文件是否支持
    """
    clear_error()
    
    try:
        if not file_path:
            set_error(b"Invalid file path")
            return 0
        
        file_str = file_path.decode('utf-8')
        file_ext = os.path.splitext(file_str)[1].lower()
        
        supported_formats = ['.pdf', '.docx']
        return 1 if file_ext in supported_formats else 0
        
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return 0

cdef public const char* get_supported_formats():
    """
    获取支持的格式列表
    """
    cdef char* result
    clear_error()
    
    try:
        formats_json = '{"pdf": "docx", "docx": "pdf"}'
        formats_bytes = formats_json.encode('utf-8')
        
        result = <char*>malloc(len(formats_bytes) + 1)
        if result:
            strcpy(result, formats_bytes)
        
        return result
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return b"{}"

cdef public const char* get_file_type(const char* file_path):
    """
    获取文件类型
    """
    clear_error()
    
    try:
        if not file_path:
            set_error(b"Invalid file path")
            return b"unknown"
        
        file_str = file_path.decode('utf-8')
        file_ext = os.path.splitext(file_str)[1].lower()
        
        if file_ext == '.pdf':
            return b"pdf"
        elif file_ext == '.docx':
            return b"docx"
        else:
            return b"unknown"
            
    except Exception as e:
        error_msg = str(e).encode('utf-8')
        set_error(error_msg)
        return b"error"

cdef public char* get_last_error():
    """
    获取最后一次错误信息
    """
    if g_error_buffer != NULL:
        return g_error_buffer
    return <char*>""

# 声明必要的C函数
cdef extern from "stdlib.h":
    void* malloc(size_t size)
    void free(void* ptr)

cdef extern from "string.h":
    char* strcpy(char* dest, char* src)