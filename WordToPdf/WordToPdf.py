"""
PDF与DOCX互转命令行工具
使用方式:
convert input_file output_file
示例:
convert document.docx output.pdf
convert file.pdf result.docx
"""

import argparse
import os
import sys
import subprocess
from pathlib import Path
from typing import Optional

# 导入转换库
try:
    from docx2pdf import convert as docx_to_pdf
    from pdf2docx import Converter
except ImportError as e:
    print(f"缺少必要的依赖库: {e}")
    print("请先安装: pip install docx2pdf pdf2docx python-docx reportlab")
    sys.exit(1)

# 备用转换方案依赖
try:
    from docx import Document
    from reportlab.pdfgen import canvas
    from reportlab.lib.pagesizes import A4
    from reportlab.pdfbase import pdfmetrics
    from reportlab.pdfbase.ttfonts import TTFont
except ImportError:
    print("警告: 备用转换库未安装，将无法使用降级方案")
    Document = None


def validate_file(file_path: str, expected_ext: list) -> bool:
    """
    验证文件是否存在且格式符合要求
    
    Args:
        file_path: 文件路径
        expected_ext: 期望的文件扩展名列表 (如 ['.docx', '.pdf'])
    
    Returns:
        bool: 验证结果
    """
    if not os.path.exists(file_path):
        print(f"错误: 文件 '{file_path}' 不存在")
        return False
    
    file_ext = Path(file_path).suffix.lower()
    if file_ext not in expected_ext:
        print(f"错误: 文件格式不支持，期望格式: {', '.join(expected_ext)}")
        return False
    
    return True


def check_word_installed() -> bool:
    """检查系统是否安装了Microsoft Word"""
    try:
        # 尝试调用Word COM组件
        import win32com.client
        win32com.client.Dispatch("Word.Application")
        return True
    except Exception:
        return False


def docx_to_pdf_fallback(input_path: str, output_path: str) -> bool:
    """
    备用的DOCX转PDF方案（使用python-docx+reportlab）
    注意：该方案仅支持纯文本转换，不支持复杂格式
    """
    if Document is None:
        print("错误: 备用转换库未安装，无法使用降级方案")
        return False
    
    try:
        print("使用备用方案转换（仅支持纯文本）...")
        doc = Document(input_path)
        c = canvas.Canvas(output_path, pagesize=A4)
        width, height = A4
        
        # 设置字体（使用系统默认字体）
        try:
            pdfmetrics.registerFont(TTFont('SimSun', 'C:/Windows/Fonts/simsun.ttc'))
            c.setFont("SimSun", 12)
        except:
            c.setFont("Helvetica", 12)
        
        # 写入文本
        y = height - 50
        line_height = 15
        for para in doc.paragraphs:
            if y < 50:  # 新建页面
                c.showPage()
                c.setFont("Helvetica", 12)
                y = height - 50
            
            c.drawString(50, y, para.text)
            y -= line_height
        
        c.save()
        print(f"备用方案转换完成！PDF文件已保存至: {output_path}")
        return True
    except Exception as e:
        print(f"备用转换方案失败: {e}")
        return False


def convert_docx_to_pdf(input_path: str, output_path: str):
    """将DOCX文件转换为PDF（带错误处理和备用方案）"""
    try:
        print(f"正在将 {input_path} 转换为 {output_path}...")
        
        # 首先尝试使用docx2pdf（依赖Word）
        try:
            docx_to_pdf(input_path, output_path)
            print(f"转换完成！PDF文件已保存至: {output_path}")
            return
        except Exception as e:
            print(f"主转换方案失败: {e}")
            
            # 尝试备用方案
            if docx_to_pdf_fallback(input_path, output_path):
                return
            
            # 所有方案都失败
            print("所有转换方案均失败，请检查：")
            print("1. 是否安装了Microsoft Word并正常运行")
            print("2. 是否以管理员身份运行该程序")
            print("3. Word进程是否被占用或损坏")
            sys.exit(1)
            
    except Exception as e:
        print(f"DOCX转PDF失败: {e}")
        sys.exit(1)


def convert_pdf_to_docx(input_path: str, output_path: str):
    """将PDF文件转换为DOCX"""
    try:
        print(f"正在将 {input_path} 转换为 {output_path}...")
        cv = Converter(input_path)
        cv.convert(output_path, start=0, end=None)
        cv.close()
        print(f"转换完成！DOCX文件已保存至: {output_path}")
    except Exception as e:
        print(f"PDF转DOCX失败: {e}")
        sys.exit(1)


def main():
    # 创建参数解析器
    parser = argparse.ArgumentParser(
        description='PDF与DOCX格式互转工具',
        prog='convert',
        usage='%(prog)s [输入文件] [输出文件]'
    )
    parser.add_argument('input_file', help='输入文件路径 (支持 .docx 或 .pdf)')
    parser.add_argument('output_file', help='输出文件路径 (支持 .pdf 或 .docx)')
    
    # 解析参数
    args = parser.parse_args()
    
    input_file = args.input_file
    output_file = args.output_file
    
    # 获取文件扩展名
    input_ext = Path(input_file).suffix.lower()
    output_ext = Path(output_file).suffix.lower()
    
    # 验证输入输出格式组合
    valid_combinations = [
        ('.docx', '.pdf'),
        ('.pdf', '.docx')
    ]
    
    if (input_ext, output_ext) not in valid_combinations:
        print("错误: 不支持的格式转换组合")
        print("支持的转换组合:")
        print("  - DOCX 转 PDF: convert input.docx output.pdf")
        print("  - PDF 转 DOCX: convert input.pdf output.docx")
        sys.exit(1)
    
    # 验证输入文件
    if not validate_file(input_file, [input_ext]):
        sys.exit(1)
    
    # 检查输出目录是否存在
    output_dir = os.path.dirname(output_file)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # 根据格式执行转换
    if input_ext == '.docx' and output_ext == '.pdf':
        # 预检查Word是否安装
        if not check_word_installed():
            print("警告: 未检测到Microsoft Word，将使用备用转换方案（仅支持纯文本）")
        convert_docx_to_pdf(input_file, output_file)
    elif input_ext == '.pdf' and output_ext == '.docx':
        convert_pdf_to_docx(input_file, output_file)


if __name__ == "__main__":
    # 以管理员身份运行建议
    if not sys.platform.startswith('win32'):
        print("警告: 该工具仅支持Windows系统")
        sys.exit(1)
    
    main()