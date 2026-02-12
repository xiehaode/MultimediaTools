#!/bin/bash

# MultiMediaTool Server 依赖安装脚本

echo "=== MultiMediaTool Server 依赖安装脚本 ==="

# 更新包列表
echo "更新包列表..."
sudo apt update

# 安装基本开发工具
echo "安装基本开发工具..."
sudo apt install -y build-essential cmake git pkg-config

# 安装SSL库
echo "安装SSL开发库..."
sudo apt install -y libssl-dev

# 安装MySQL/MariaDB开发库
echo "安装MySQL/MariaDB开发库..."
sudo apt install -y libmysqlclient-dev

# 安装Boost库
echo "安装Boost开发库..."
sudo apt install -y libboost-all-dev

# 尝试安装cpprestsdk
echo "尝试安装cpprestsdk..."
if apt-cache show libcpprest-dev > /dev/null 2>&1; then
    sudo apt install -y libcpprest-dev
    echo "✓ cpprestsdk 安装成功"
else
    echo "❌ cpprestsdk 在仓库中不可用，将使用简化版HTTP服务器"
    echo "如需完整功能，请手动安装 cpprestsdk："
    echo "  方法1: git clone https://github.com/microsoft/cpprestsdk && 编译安装"
    echo "  方法2: 使用vcpkg: vcpkg install cpprestsdk"
fi

# 安装MySQL服务器（如果尚未安装）
echo "检查MySQL服务器..."
if ! command -v mysql &> /dev/null; then
    echo "安装MySQL服务器..."
    sudo apt install -y mysql-server
    echo "请运行 'sudo mysql_secure_installation' 来配置MySQL"
else
    echo "✓ MySQL已安装"
fi

echo ""
echo "=== 依赖安装完成 ==="
echo ""
echo "下一步："
echo "1. 启动MySQL服务: sudo systemctl start mysql"
echo "2. 创建数据库: sudo mysql -e 'CREATE DATABASE multimediatool;'"
echo "3. 导入表结构: mysql -u root -p multimediatool < mysql_schema.sql"
echo "4. 编译服务器: ./build.sh"
echo ""
echo "如果遇到权限问题，请确保当前用户有访问MySQL的权限"