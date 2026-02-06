
"""
简单的Word转PDF包装器 - 使用ctypes调用C++ DLL
"""

import ctypes
import os
from pathlib import Path

class WordToPdfConverter:
    def __init__(self):
        self.dll_path = None
        self.dll = None
        self._load_dll()
    
    def _load_dll(self):
        """加载DLL文件"""
        # 尝试不同的路径
        possible_paths = [
            "bin/x64/Release/WordToPdfNative.dll",
            "../bin/x64/Release/WordToPdfNative.dll",
            "../../bin/x64/Release/WordToPdfNative.dll",
            "WordToPdfNative.dll",
        ]
        
        for path in possible_paths:
            if os.path.exists(path):
                self.dll_path = path
                break
        
        if not self.dll_path:
            raise FileNotFoundError("找不到WordToPdfNative.dll文件")
        
        self.dll = ctypes.CDLL(self.dll_path)
        
        # 设置函数签名
        self.dll.ConvertWordToPdf.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self.dll.ConvertWordToPdf.restype = ctypes.c_int
        
        self.dll.ConvertPdfToWord.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self.dll.ConvertPdfToWord.restype = ctypes.c_int
        
        self.dll.IsWordInstalled.argtypes = []
        self.dll.IsWordInstalled.restype = ctypes.c_int
        
        self.dll.GetLastError.argtypes = []
        self.dll.GetLastError.restype = ctypes.c_char_p
        
        self.dll.ClearLastError.argtypes = []
        self.dll.ClearLastError.restype = None
    
    def convert_word_to_pdf(self, input_path, output_path):
        """将Word文档转换为PDF"""
        if not os.path.exists(input_path):
            raise FileNotFoundError(f"输入文件不存在: {input_path}")
        
        self.dll.ClearLastError()
        
        input_bytes = input_path.encode('utf-8')
        output_bytes = output_path.encode('utf-8')
        
        result = self.dll.ConvertWordToPdf(input_bytes, output_bytes)
        
        if result != 0:
            return True
        else:
            error = self.dll.GetLastError()
            if error:
                raise Exception(f"转换失败: {error.decode('utf-8')}")
            else:
                raise Exception("转换失败: 未知错误")
    
    def convert_pdf_to_word(self, input_path, output_path):
        """将PDF文档转换为Word"""
        if not os.path.exists(input_path):
            raise FileNotFoundError(f"输入文件不存在: {input_path}")
        
        self.dll.ClearLastError()
        
        input_bytes = input_path.encode('utf-8')
        output_bytes = output_path.encode('utf-8')
        
        result = self.dll.ConvertPdfToWord(input_bytes, output_bytes)
        
        if result != 0:
            return True
        else:
            error = self.dll.GetLastError()
            if error:
                raise Exception(f"转换失败: {error.decode('utf-8')}")
            else:
                raise Exception("转换失败: 未知错误")
    
    def is_word_installed(self):
        """检查是否安装了Microsoft Word"""
        return self.dll.IsWordInstalled() != 0

# 使用示例
if __name__ == "__main__":
    try:
        converter = WordToPdfConverter()
        
        print(f"Word安装状态: {'已安装' if converter.is_word_installed() else '未安装'}")
        
        # 示例转换
        # converter.convert_word_to_pdf("test.docx", "output.pdf")
        # converter.convert_pdf_to_word("input.pdf", "output.docx")
        
        print("包装器初始化成功!")
        
    except Exception as e:
        print(f"错误: {e}")
