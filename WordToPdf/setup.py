"""
setup.py - 用于构建和打包通用文件格式转换器
"""

from setuptools import setup, Extension, find_packages
from Cython.Build import cythonize
import numpy
import sys
import os

# 编译选项
compile_args = ['-O2', '-Wall']
link_args = []

# Windows特定设置
if sys.platform == 'win32':
    compile_args.extend(['/EHsc', '/bigobj'])
    link_args.extend(['/LIBPATH:' + os.path.join(numpy.get_include(), '..', 'lib')])
else:
    compile_args.extend(['-fPIC', '-O3'])
    link_args.extend(['-fPIC'])

# 定义扩展模块
extensions = [
    Extension(
        "converter_wrapper",
        sources=["converter_wrapper_simple.pyx"],
        include_dirs=[numpy.get_include()],
        extra_compile_args=compile_args,
        extra_link_args=link_args,
        language="c",
    )
]

# 依赖包
install_requires = [
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
    'Cython>=0.29.0',
    'numpy>=1.20.0',
]

# 额外依赖（可选）
extras_require = {
    'dev': [
        'pytest>=6.0.0',
        'pytest-cov>=2.0.0',
        'black>=21.0.0',
        'flake8>=3.8.0',
    ],
    'gui': [
        'PyQt5>=5.15.0',
    ]
}

setup(
    name="universal-file-converter",
    version="1.0.0",
    author="Your Name",
    author_email="your.email@example.com",
    description="通用文件格式转换器 - 支持PDF、Word、HTML、PPT、图片、CSV互转",
    long_description=open('README.md', 'r', encoding='utf-8').read() if os.path.exists('README.md') else "",
    long_description_content_type="text/markdown",
    url="https://github.com/yourusername/universal-file-converter",
    packages=find_packages(),
    ext_modules=cythonize(
        extensions,
        compiler_directives={
            'language_level': "3",
            'embedsignature': True,
        }
    ),
    py_modules=['universal_converter', 'converter_cli'],
    install_requires=install_requires,
    extras_require=extras_require,
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Cython",
        "Programming Language :: C",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Multimedia :: Graphics :: Conversion",
        "Topic :: Text Processing :: Markup",
        "Topic :: Office/Business",
    ],
    keywords="file converter pdf word html ppt image csv format conversion",
    python_requires='>=3.8',
    include_package_data=True,
    package_data={
        '': ['*.pyx', '*.pxd', '*.h', '*.c'],
    },
    entry_points={
        'console_scripts': [
            'universal-converter=converter_cli:main',
        ],
    },
    zip_safe=False,
)