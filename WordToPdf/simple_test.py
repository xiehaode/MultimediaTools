"""
简单的WordToPdf测试 - 不依赖额外库
"""

import sys
import os
from pathlib import Path

def test_basic_conversion():
    """测试基本转换功能"""
    print("WordToPdf 基本功能测试")
    print("=" * 30)
    
    # 测试原始Python工具
    try:
        print("1. 测试原始Python工具...")
        sys.path.append('.')
        from WordToPdf import main as word_main
        
        print("   WordToPdf.py 导入成功")
        print("   支持的功能:")
        print("   - DOCX 转 PDF")
        print("   - PDF 转 DOCX")
        
    except ImportError as e:
        print(f"   WordToPdf.py 导入失败: {e}")
    
    # 测试我们的包装器
    try:
        print("\n2. 测试ctypes包装器...")
        from word_converter_ctypes import convert_file, get_supported_formats, get_file_type
        
        print("   word_converter_ctypes.py 导入成功")
        
        # 测试支持的格式
        formats = get_supported_formats()
        print(f"   支持的格式: {formats}")
        
        # 测试文件类型检测
        test_file = "test.docx"
        file_type = get_file_type(test_file)
        print(f"   {test_file} -> {file_type}")
        
        # 如果有测试文件，尝试转换
        if os.path.exists("test.docx"):
            print("\n3. 尝试转换 test.docx...")
            result = convert_file("test.docx", "test_output.pdf")
            if result:
                print("   转换成功!")
                if os.path.exists("test_output.pdf"):
                    print(f"   输出文件: {os.path.abspath('test_output.pdf')}")
            else:
                from word_converter_ctypes import get_last_error
                error = get_last_error()
                if isinstance(error, bytes):
                    error = error.decode('utf-8')
                print(f"   转换失败: {error}")
        else:
            print("\n3. 没有找到 test.docx，跳过转换测试")
            print("   提示: 可以运行 'python test_converter.py input.docx output.pdf' 进行测试")
        
    except ImportError as e:
        print(f"   word_converter_ctypes.py 导入失败: {e}")
    
    print("\n测试完成!")

def show_usage():
    """显示使用说明"""
    print("\n使用说明:")
    print("1. 原始Python工具:")
    print("   python WordToPdf.py input.docx output.pdf")
    print("   python WordToPdf.py input.pdf output.docx")
    
    print("\n2. 包装器接口:")
    print("   from word_converter_ctypes import convert_file")
    print("   convert_file('input.docx', 'output.pdf')")
    
    print("\n3. 命令行测试:")
    print("   python test_converter.py input.docx output.pdf")

if __name__ == "__main__":
    test_basic_conversion()
    show_usage()