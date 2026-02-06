"""
WordToPdf Native Converter - 纯Python实现
使用现有的Python转换工具提供C风格的接口
"""

import os
import sys
import ctypes
from pathlib import Path
from typing import Optional, List

# 尝试导入转换函数
try:
    from WordToPdf import convert_docx_to_pdf, convert_pdf_to_docx, main as word_main
    HAS_NATIVE_CONVERTER = True
except ImportError:
    HAS_NATIVE_CONVERTER = False
    print("警告: 无法导入WordToPdf转换器，将提供模拟接口")

class WordToPdfNativeAPI:
    """模拟C API的Python包装器"""
    
    def __init__(self):
        self._last_error = ""
    
    def convert_file(self, input_path: str, output_path: str) -> int:
        """
        模拟C函数: int convert_file(const char* input_path, const char* output_path)
        返回: 1表示成功，0表示失败
        """
        try:
            self._last_error = ""
            
            if not os.path.exists(input_path):
                self._last_error = f"输入文件不存在: {input_path}"
                return 0
            
            input_ext = Path(input_path).suffix.lower()
            output_ext = Path(output_path).suffix.lower()
            
            if not HAS_NATIVE_CONVERTER:
                self._last_error = "转换器模块未安装"
                return 0
            
            # 根据文件类型进行转换
            if input_ext == '.docx' and output_ext == '.pdf':
                convert_docx_to_pdf(input_path, output_path)
                return 1
            elif input_ext == '.pdf' and output_ext == '.docx':
                convert_pdf_to_docx(input_path, output_path)
                return 1
            else:
                self._last_error = f"不支持的转换: {input_ext} -> {output_ext}"
                return 0
                
        except Exception as e:
            self._last_error = str(e)
            return 0
    
    def convert_files_batch(self, input_paths: List[str], output_dir: str, output_format: str) -> int:
        """
        模拟C函数: int convert_files_batch(const char* const* input_paths, int file_count, ...)
        返回: 1表示成功，0表示失败
        """
        try:
            self._last_error = ""
            
            if not os.path.exists(output_dir):
                os.makedirs(output_dir, exist_ok=True)
            
            success_count = 0
            for input_path in input_paths:
                input_path = input_path.strip('"')  # 移除可能的引号
                
                if not os.path.exists(input_path):
                    continue
                
                # 生成输出文件名
                input_name = Path(input_path).stem
                output_path = os.path.join(output_dir, f"{input_name}.{output_format}")
                
                # 转换文件
                if self.convert_file(input_path, output_path):
                    success_count += 1
            
            return 1 if success_count > 0 else 0
            
        except Exception as e:
            self._last_error = str(e)
            return 0
    
    def check_file_support(self, file_path: str) -> int:
        """
        模拟C函数: int check_file_support(const char* file_path)
        返回: 1表示支持，0表示不支持
        """
        try:
            self._last_error = ""
            
            if not os.path.exists(file_path):
                self._last_error = f"文件不存在: {file_path}"
                return 0
            
            file_ext = Path(file_path).suffix.lower()
            supported_formats = ['.pdf', '.docx']
            
            return 1 if file_ext in supported_formats else 0
            
        except Exception as e:
            self._last_error = str(e)
            return 0
    
    def get_supported_formats(self) -> str:
        """
        模拟C函数: const char* get_supported_formats()
        返回: JSON格式的支持格式
        """
        self._last_error = ""
        formats = {
            "pdf": {"description": "Portable Document Format", "can_convert_to": ["docx"]},
            "docx": {"description": "Microsoft Word Document", "can_convert_to": ["pdf"]}
        }
        import json
        return json.dumps(formats, ensure_ascii=False)
    
    def get_file_type(self, file_path: str) -> str:
        """
        模拟C函数: const char* get_file_type(const char* file_path)
        返回: 文件类型字符串
        """
        try:
            self._last_error = ""
            
            if not os.path.exists(file_path):
                self._last_error = f"文件不存在: {file_path}"
                return "error"
            
            file_ext = Path(file_path).suffix.lower()
            type_map = {
                '.pdf': 'pdf',
                '.docx': 'docx',
                '.doc': 'doc',
                '.txt': 'text',
                '.rtf': 'rtf'
            }
            
            return type_map.get(file_ext, 'unknown')
            
        except Exception as e:
            self._last_error = str(e)
            return "error"
    
    def get_last_error(self) -> str:
        """
        模拟C函数: const char* get_last_error()
        返回: 最后一次错误信息
        """
        return self._last_error
    
    def clear_error(self):
        """
        模拟C函数: void clear_error()
        清除错误信息
        """
        self._last_error = ""

# 全局API实例
_api_instance = WordToPdfNativeAPI()

# 导出C风格函数
def convert_file(input_path: str, output_path: str) -> int:
    """C风格接口函数"""
    return _api_instance.convert_file(input_path, output_path)

def convert_files_batch(input_paths: List[str], output_dir: str, output_format: str) -> int:
    """C风格接口函数"""
    return _api_instance.convert_files_batch(input_paths, output_dir, output_format)

def check_file_support(file_path: str) -> int:
    """C风格接口函数"""
    return _api_instance.check_file_support(file_path)

def get_supported_formats() -> str:
    """C风格接口函数"""
    return _api_instance.get_supported_formats()

def get_file_type(file_path: str) -> str:
    """C风格接口函数"""
    return _api_instance.get_file_type(file_path)

def get_last_error() -> str:
    """C风格接口函数"""
    return _api_instance.get_last_error()

def clear_error():
    """C风格接口函数"""
    _api_instance.clear_error()

# 命令行接口
def main():
    """命令行主函数"""
    if len(sys.argv) < 3:
        print("用法: python word_converter_native.py <输入文件> <输出文件>")
        print("示例: python word_converter_native.py input.docx output.pdf")
        return 1
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    print(f"正在转换 {input_file} -> {output_file}...")
    
    if convert_file(input_file, output_file):
        print("转换成功!")
        return 0
    else:
        error = get_last_error()
        print(f"转换失败: {error}")
        return 1

if __name__ == "__main__":
    sys.exit(main())