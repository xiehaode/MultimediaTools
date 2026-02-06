"""
一键安装依赖和构建DLL的脚本
"""

import os
import sys
import subprocess
import platform

def run_command(command, description):
    """运行命令并显示结果"""
    print(f"\n{'='*50}")
    print(f"执行: {description}")
    print(f"命令: {command}")
    print('='*50)
    
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True)
        
        if result.stdout:
            print(result.stdout)
        
        if result.stderr:
            print("错误信息:")
            print(result.stderr)
        
        return result.returncode == 0
    except Exception as e:
        print(f"执行失败: {e}")
        return False

def check_python_version():
    """检查Python版本"""
    print(f"Python版本: {sys.version}")
    if sys.version_info < (3, 8):
        print("错误: 需要Python 3.8或更高版本")
        return False
    return True

def install_dependencies():
    """安装依赖包"""
    print("\n开始安装依赖包...")
    
    # 首先升级pip
    success = run_command(
        f"{sys.executable} -m pip install --upgrade pip",
        "升级pip"
    )
    
    if not success:
        print("警告: pip升级失败，继续安装...")
    
    # 安装requirements.txt中的依赖
    if os.path.exists('requirements.txt'):
        success = run_command(
            f"{sys.executable} -m pip install -r requirements.txt",
            "安装requirements.txt中的依赖"
        )
        if not success:
            return False
    
    # 额外安装必要的构建依赖
    build_deps = [
        "wheel",
        "setuptools>=50.0.0",
    ]
    
    for dep in build_deps:
        run_command(
            f"{sys.executable} -m pip install {dep}",
            f"安装 {dep}"
        )
    
    return True

def check_system_dependencies():
    """检查系统依赖"""
    system = platform.system().lower()
    
    print(f"\n检查系统依赖 (操作系统: {system})")
    
    if system == "windows":
        print("Windows系统检查:")
        print("注意: 以下为可选但推荐的系统依赖:")
        print("1. Microsoft Word (用于更好的Word转PDF效果)")
        print("2. LibreOffice (用于PPT转PDF)")
        print("3. Poppler (用于PDF转图片)")
        print("请从官方网站下载安装这些依赖以获得最佳效果")
        
    elif system == "linux":
        print("Linux系统检查:")
        success = run_command(
            "which soffice || which libreoffice",
            "检查LibreOffice"
        )
        
        if not success:
            print("警告: 未找到LibreOffice，PPT转PDF功能可能受限")
            print("请运行: sudo apt-get install libreoffice poppler-utils")
        else:
            print("✓ 找到LibreOffice")
        
        success = run_command("which pdftoppm", "检查Poppler")
        if not success:
            print("警告: 未找到Poppler，PDF转图片功能可能受限")
            print("请运行: sudo apt-get install poppler-utils")
        else:
            print("✓ 找到Poppler")
    
    return True

def test_imports():
    """测试关键库导入"""
    print("\n测试关键库导入...")
    
    critical_libs = [
        "pandas",
        "PIL", 
        "docx",
        "bs4",
        "pdfplumber",
        "reportlab",
        "fpdf",
        "Cython",
        "numpy"
    ]
    
    success_count = 0
    
    for lib in critical_libs:
        try:
            __import__(lib)
            print(f"✓ {lib}")
            success_count += 1
        except ImportError as e:
            print(f"✗ {lib}: {e}")
    
    print(f"\n导入测试完成: {success_count}/{len(critical_libs)} 成功")
    return success_count >= len(critical_libs) * 0.8  # 80%成功率

def build_dll():
    """构建DLL"""
    print("\n开始构建DLL...")
    
    if not os.path.exists('build_dll.py'):
        print("错误: 未找到build_dll.py文件")
        return False
    
    success = run_command(
        f"{sys.executable} build_dll.py",
        "构建DLL"
    )
    
    if success:
        print("\n✓ DLL构建成功!")
        print("构建文件位置:")
        
        release_dir = 'release'
        if os.path.exists(release_dir):
            print(f"  - 目录: {os.path.abspath(release_dir)}")
            for file in os.listdir(release_dir):
                if file.endswith(('.dll', '.pyd', '.so')):
                    print(f"  - {file}")
    
    return success

def run_test():
    """运行测试"""
    print("\n运行转换器测试...")
    
    if not os.path.exists('test_converter.py'):
        print("警告: 未找到测试文件")
        return True
    
    success = run_command(
        f"{sys.executable} test_converter.py",
        "运行功能测试"
    )
    
    return success

def main():
    """主函数"""
    print("通用文件转换器 - 一键安装和构建脚本")
    print("="*60)
    
    # 检查Python版本
    if not check_python_version():
        sys.exit(1)
    
    # 安装依赖
    if not install_dependencies():
        print("依赖安装失败，请检查错误信息")
        sys.exit(1)
    
    # 检查系统依赖
    check_system_dependencies()
    
    # 测试导入
    if not test_imports():
        print("警告: 部分依赖导入失败，可能影响转换功能")
    
    # 构建DLL
    if not build_dll():
        print("DLL构建失败")
        sys.exit(1)
    
    # 运行测试
    print("\n是否运行功能测试? (y/n): ", end="")
    try:
        choice = input().lower().strip()
        if choice in ['y', 'yes', '是']:
            run_test()
    except:
        print("跳过测试")
    
    print("\n" + "="*60)
    print("安装和构建完成!")
    print("\n下一步:")
    print("1. 查看README.md了解详细使用方法")
    print("2. 在Qt项目中集成DLL (参考release/qt_converter.h)")
    print("3. 使用命令行工具: python converter_cli.py --help")
    print("4. 在Python代码中导入使用: from universal_converter import UniversalConverter")
    print("="*60)

if __name__ == "__main__":
    main()