
"""
WordToPdf ctypes包装器 - 模拟C DLL行为
"""

import ctypes
import os
import sys
from pathlib import Path

class MockWordToPdfDLL:
    """模拟WordToPdf DLL的行为"""
    
    def __init__(self):
        self._last_error = ""
        
    def convert_file(self, input_path, output_path):
        """模拟 convert_file 函数"""
        try:
            # 导入实际的转换函数
            from WordToPdf import convert_docx_to_pdf, convert_pdf_to_docx
            
            self._last_error = ""
            
            if not os.path.exists(input_path):
                self._last_error = f"输入文件不存在: {input_path}"
                return 0
            
            input_ext = Path(input_path).suffix.lower()
            output_ext = Path(output_path).suffix.lower()
            
            if input_ext == '.docx' and output_ext == '.pdf':
                convert_docx_to_pdf(input_path, output_path)
                return 1
            elif input_ext == '.pdf' and output_ext == '.docx':
                convert_pdf_to_docx(input_path, output_path)
                return 1
            else:
                self._last_error = f"不支持的格式转换: {input_ext} -> {output_ext}"
                return 0
                
        except ImportError:
            self._last_error = "WordToPdf模块未找到"
            return 0
        except Exception as e:
            self._last_error = str(e)
            return 0
    
    def convert_files_batch(self, input_paths, file_count, output_dir, output_format):
        """模拟批量转换函数"""
        try:
            self._last_error = ""
            
            # 确保输出目录存在
            os.makedirs(output_dir, exist_ok=True)
            
            success_count = 0
            for i in range(file_count):
                input_path = input_paths[i].decode('utf-8') if isinstance(input_paths[i], bytes) else input_paths[i]
                
                if not os.path.exists(input_path):
                    continue
                
                # 生成输出文件名
                input_name = Path(input_path).stem
                output_path = os.path.join(output_dir, f"{input_name}.{output_format}")
                
                if self.convert_file(input_path, output_path):
                    success_count += 1
            
            return 1 if success_count > 0 else 0
            
        except Exception as e:
            self._last_error = str(e)
            return 0
    
    def check_file_support(self, file_path):
        """检查文件支持"""
        try:
            self._last_error = ""
            
            if not os.path.exists(file_path):
                self._last_error = f"文件不存在: {file_path}"
                return 0
            
            file_ext = Path(file_path).suffix.lower()
            supported = ['.pdf', '.docx']
            return 1 if file_ext in supported else 0
            
        except Exception as e:
            self._last_error = str(e)
            return 0
    
    def get_supported_formats(self):
        """获取支持的格式"""
        self._last_error = ""
        return '{"pdf": "docx", "docx": "pdf"}'
    
    def get_file_type(self, file_path):
        """获取文件类型"""
        try:
            self._last_error = ""
            
            if not os.path.exists(file_path):
                self._last_error = f"文件不存在: {file_path}"
                return "error"
            
            file_ext = Path(file_path).suffix.lower()
            type_map = {
                '.pdf': 'pdf',
                '.docx': 'docx',
                '.doc': 'doc'
            }
            return type_map.get(file_ext, 'unknown')
            
        except Exception as e:
            self._last_error = str(e)
            return "error"
    
    def get_last_error(self):
        """获取最后错误"""
        return self._last_error.encode('utf-8')
    
    def clear_error(self):
        """清除错误"""
        self._last_error = ""

# 全局模拟DLL实例
_mock_dll = MockWordToPdfDLL()

# C函数接口
def convert_file(input_path, output_path):
    return _mock_dll.convert_file(input_path, output_path)

def convert_files_batch(input_paths, file_count, output_dir, output_format):
    return _mock_dll.convert_files_batch(input_paths, file_count, output_dir, output_format)

def check_file_support(file_path):
    return _mock_dll.check_file_support(file_path)

def get_supported_formats():
    return _mock_dll.get_supported_formats()

def get_file_type(file_path):
    return _mock_dll.get_file_type(file_path)

def get_last_error():
    return _mock_dll.get_last_error()

def clear_error():
    _mock_dll.clear_error()
