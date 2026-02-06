"""
简化的构建脚本 - 跳过复杂的Cython构建，直接使用现有的C++项目
"""

import os
import sys
import subprocess
from pathlib import Path

def build_native_dll():
    """构建Native C++ DLL"""
    print("开始构建Native C++ DLL...")
    
    # 检查项目文件是否存在
    project_file = Path("WordToPdfNative.vcxproj")
    if not project_file.exists():
        print(f"错误: 找不到项目文件 {project_file}")
        return False
    
    print(f"找到项目文件: {project_file}")
    
    # 尝试使用MSBuild构建
    try:
        print("尝试使用MSBuild构建...")
        
        # 查找MSBuild
        msbuild_paths = [
            r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
            r"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
        ]
        
        msbuild_exe = None
        for path in msbuild_paths:
            if os.path.exists(path):
                msbuild_exe = path
                break
        
        if not msbuild_exe:
            print("错误: 找不到MSBuild，请安装Visual Studio")
            print("或者尝试在Visual Studio中手动构建项目")
            return False
        
        print(f"使用MSBuild: {msbuild_exe}")
        
        # 构建项目
        result = subprocess.run([
            msbuild_exe,
            "WordToPdfNative.vcxproj",
            "/p:Configuration=Release",
            "/p:Platform=x64",
            "/p:OutDir=bin\\x64\\Release\\",
            "/verbosity:minimal"
        ], cwd=Path.cwd())
        
        if result.returncode == 0:
            print("构建成功!")
            
            # 查找生成的文件
            dll_path = Path("bin/x64/Release/WordToPdfNative.dll")
            if dll_path.exists():
                print(f"DLL文件位置: {dll_path.absolute()}")
                return True
            else:
                print("警告: 构建成功但找不到DLL文件")
                return False
        else:
            print(f"构建失败，返回码: {result.returncode}")
            return False
            
    except Exception as e:
        print(f"构建过程中出现错误: {e}")
        return False

def create_simple_wrapper():
    """创建简单的Python包装器"""
    print("创建简单的Python包装器...")
    
    wrapper_code = '''
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
        
        self.dll.WordToPdf_GetLastError.argtypes = []
        self.dll.WordToPdf_GetLastError.restype = ctypes.c_char_p
        
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
            error = self.dll.WordToPdf_GetLastError()
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
            error = self.dll.WordToPdf_GetLastError()
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
'''
    
    with open("word_converter.py", "w", encoding="utf-8") as f:
        f.write(wrapper_code)
    
    print("创建了简单的Python包装器: word_converter.py")
    return True

def main():
    """主函数"""
    print("WordToPdf 简化构建脚本")
    print("=" * 50)
    
    # 构建C++ DLL
    success = build_native_dll()
    
    if not success:
        print("C++ DLL构建失败，尝试创建Python包装器...")
        create_simple_wrapper()
        print("\n建议:")
        print("1. 在Visual Studio中打开MultiMediaTool.sln")
        print("2. 手动构建WordToPdfNative项目")
        print("3. 运行python word_converter.py测试")
        return False
    
    # 创建Python包装器
    create_simple_wrapper()
    
    print("\n构建完成!")
    print("使用方法:")
    print("1. 运行 python word_converter.py")
    print("2. 在其他项目中导入 word_converter")
    
    return True

if __name__ == "__main__":
    success = main()
    if not success:
        sys.exit(1)