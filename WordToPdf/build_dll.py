"""
构建DLL脚本 - 将Python转换器编译为DLL供Qt使用
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

def build_dll():
    """构建DLL文件"""
    print("开始构建DLL...")
    
    # 检查依赖
    try:
        import Cython
        print(f"Cython版本: {Cython.__version__}")
    except ImportError:
        print("错误: 请先安装Cython")
        print("运行: pip install cython")
        print("或尝试: python -m pip install cython")
        print("当前Python路径:", sys.executable)
        return False
    
    try:
        import numpy
        print(f"NumPy版本: {numpy.__version__}")
    except ImportError:
        print("错误: 请先安装NumPy")
        print("运行: pip install numpy")
        return False
    
    # 清理之前的构建
    print("清理之前的构建文件...")
    for dir_name in ['build', 'dist', '__pycache__']:
        if os.path.exists(dir_name):
            shutil.rmtree(dir_name)
    
    # 清理临时文件
    keep_files = {
        'setup.py', 'build_dll.py',
        'converter_wrapper.c', 'converter_wrapper_simple.c'
    }
    for ext in ['.so', '.pyd', '.dll', '.c', '.html']:
        for file in Path('.').glob(f'*{ext}'):
            if file.name not in keep_files:
                file.unlink()
    
    # 构建扩展模块
    print("构建Cython扩展...")
    script_dir = os.path.dirname(os.path.abspath(__file__))
    setup_path = os.path.join(script_dir, 'setup.py')
    
    try:
        result = subprocess.run([
            sys.executable, setup_path, 'build_ext', '--inplace'
        ], cwd=script_dir, capture_output=True, text=True)
        
        if result.returncode != 0:
            print("构建失败:")
            print(result.stderr)
            return False
        
        print("构建成功!")
        
    except Exception as e:
        print(f"构建过程中出现错误: {e}")
        return False
    
    # 查找生成的文件
    print("查找生成的文件...")
    generated_files = []
    
    for file in Path('.').glob('*.pyd'):
        generated_files.append(file)
        print(f"找到: {file}")
    
    for file in Path('.').glob('*.dll'):
        generated_files.append(file)
        print(f"找到: {file}")
    
    if not generated_files:
        print("警告: 未找到生成的DLL文件")
        return False
    
    # 创建发布目录
    release_dir = Path('release')
    release_dir.mkdir(exist_ok=True)
    
    # 复制文件到发布目录
    print("复制文件到发布目录...")
    required_files = [
        'universal_converter.py',
        'converter_wrapper.h',
        'converter_wrapper.c',
    ]
    
    for file in required_files:
        if os.path.exists(file):
            shutil.copy2(file, release_dir)
            print(f"复制: {file}")
    
    # 复制生成的DLL
    for dll_file in generated_files:
        shutil.copy2(dll_file, release_dir)
        print(f"复制DLL: {dll_file}")
    
    # 复制依赖库
    print("复制Python依赖...")
    
    # 创建requirements.txt
    requirements = [
        'pandas>=1.3.0',
        'openpyxl>=3.0.0',
        'python-docx>=0.8.11',
        'pillow>=8.0.0',
        'PyPDF2>=2.0.0',
        'pdf2docx>=0.5.0',
        'pdfplumber>=0.6.0',
        'reportlab>=3.6.0',
        'fpdf2>=2.5.0',
        'python-pptx>=0.6.0',
        'beautifulsoup4>=4.9.0',
        'html5lib>=1.1',
        'img2pdf>=0.4.0',
        'pdf2image>=1.16.0',
        'numpy>=1.20.0',
        'Cython>=0.29.0',
    ]
    
    with open(release_dir / 'requirements.txt', 'w') as f:
        f.write('\n'.join(requirements))
    
    # 创建Qt集成示例
    qt_example = '''
#ifndef QT_CONVERTER_H
#define QT_CONVERTER_H

#include <QString>
#include <QStringList>
#include <QDebug>

// 声明C函数
extern "C" {
    int convert_file(const char* input_path, const char* output_path);
    int convert_files_batch(const char* const* input_paths, int file_count, 
                           const char* output_dir, const char* output_format);
    int check_file_support(const char* file_path);
    const char* get_supported_formats();
    const char* get_file_type(const char* file_path);
    char* get_last_error();
    void clear_error();
}

class QtFileConverter : public QObject
{
    Q_OBJECT

public:
    explicit QtFileConverter(QObject *parent = nullptr) : QObject(parent) {}

    bool convertFile(const QString &inputPath, const QString &outputPath) {
        clear_error();
        QByteArray inputBytes = inputPath.toUtf8();
        QByteArray outputBytes = outputPath.toUtf8();
        
        int result = convert_file(inputBytes.constData(), outputBytes.constData());
        
        if (!result) {
            m_lastError = QString::fromUtf8(get_last_error());
            qWarning() << "转换失败:" << m_lastError;
        }
        
        return result == 1;
    }

    bool isFileSupported(const QString &filePath) {
        clear_error();
        QByteArray pathBytes = filePath.toUtf8();
        
        int result = check_file_support(pathBytes.constData());
        
        if (!result) {
            m_lastError = QString::fromUtf8(get_last_error());
        }
        
        return result == 1;
    }

    QString getFileType(const QString &filePath) {
        clear_error();
        QByteArray pathBytes = filePath.toUtf8();
        
        const char* type = get_file_type(pathBytes.constData());
        return QString::fromUtf8(type);
    }

    QString getSupportedFormats() {
        clear_error();
        const char* formats = get_supported_formats();
        return QString::fromUtf8(formats);
    }

    QString getLastError() const {
        return m_lastError;
    }

private:
    QString m_lastError;
};

#endif // QT_CONVERTER_H
'''
    
    with open(release_dir / 'qt_converter.h', 'w') as f:
        f.write(qt_example)
    
    print(f"\n构建完成! 文件已复制到 {release_dir.absolute()}")
    print("\n在Qt项目中使用方法:")
    print("1. 包含头文件: #include \"qt_converter.h\"")
    print("2. 链接DLL文件")
    print("3. 创建转换器实例: QtFileConverter converter;")
    print("4. 调用转换方法: converter.convertFile(input, output);")
    
    return True

if __name__ == "__main__":
    if sys.platform != 'win32':
        print("警告: 此脚本主要为Windows设计，在其他平台上可能需要调整")
    
    success = build_dll()
    if not success:
        sys.exit(1)