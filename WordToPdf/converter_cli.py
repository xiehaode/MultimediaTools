"""
命令行工具 - 通用文件格式转换器
"""

import argparse
import sys
from pathlib import Path
import json
from universal_converter import UniversalConverter


def main():
    """主函数"""
    parser = argparse.ArgumentParser(
        description='通用文件格式转换器',
        prog='universal_converter',
        usage='%(prog)s [选项]'
    )
    
    # 基本转换参数
    parser.add_argument('-i', '--input', help='输入文件路径')
    parser.add_argument('-o', '--output', help='输出文件路径')
    parser.add_argument('-f', '--format', help='输出格式 (pdf, word, html, ppt, image, csv)')
    
    # 批量转换参数
    parser.add_argument('-b', '--batch', nargs='+', help='批量转换文件列表')
    parser.add_argument('-d', '--dir', help='批量转换输出目录')
    
    # 其他选项
    parser.add_argument('--list-formats', action='store_true', help='列出支持的转换格式')
    parser.add_argument('--supported', action='store_true', help='显示支持的文件扩展名')
    
    args = parser.parse_args()
    
    # 创建转换器实例
    converter = UniversalConverter()
    
    # 列出支持的格式
    if args.list_formats:
        conversions = converter.get_supported_conversions()
        print("支持的转换格式:")
        print("-" * 40)
        for from_type, to_types in conversions.items():
            print(f"{from_type.upper()} -> {', '.join(to_types)}")
        return
    
    if args.supported:
        print("支持的文件格式:")
        print("-" * 30)
        for format_name, extensions in converter.SUPPORTED_FORMATS.items():
            print(f"{format_name.upper()}: {', '.join(extensions)}")
        return
    
    # 单文件转换
    if args.input and args.output:
        print(f"正在转换: {args.input} -> {args.output}")
        success = converter.convert(args.input, args.output)
        if success:
            print("转换成功！")
        else:
            print("转换失败！")
            sys.exit(1)
    
    # 指定格式转换
    elif args.input and args.format:
        input_path = args.input
        output_ext = converter.SUPPORTED_FORMATS.get(args.format, [''])[0]
        if not output_ext:
            print(f"不支持的格式: {args.format}")
            sys.exit(1)
        
        output_path = input_path.replace(Path(input_path).suffix, output_ext)
        print(f"正在转换: {input_path} -> {output_path}")
        success = converter.convert(input_path, output_path)
        if success:
            print("转换成功！")
        else:
            print("转换失败！")
            sys.exit(1)
    
    # 批量转换
    elif args.batch and args.dir and args.format:
        print(f"批量转换 {len(args.batch)} 个文件到 {args.format} 格式...")
        results = converter.convert_batch(args.batch, args.dir, args.format)
        
        success_count = sum(1 for success in results.values() if success)
        print(f"转换完成: {success_count}/{len(results)} 成功")
        
        # 显示失败的文件
        failed_files = [file for file, success in results.items() if not success]
        if failed_files:
            print("失败的文件:")
            for file in failed_files:
                print(f"  - {file}")
    
    else:
        parser.print_help()


if __name__ == "__main__":
    main()