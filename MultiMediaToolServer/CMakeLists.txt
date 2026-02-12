# 指定 CMake 最低版本
cmake_minimum_required(VERSION 3.10)

# 项目名称（自定义）
project(MultiMediaToolServer)

# 设置 C++ 标准（C++11 及以上即可）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========== 手动指定 MySQL 路径（已存在） ==========
set(MYSQL_INCLUDE_DIR "/usr/include/mysql")
set(MYSQL_LIB "/usr/lib/x86_64-linux-gnu/libmysqlclient.so")

# 检查 MySQL 库是否找到
if(NOT MYSQL_LIB OR NOT MYSQL_INCLUDE_DIR)
    message(FATAL_ERROR "未找到 MySQL 开发库，请安装 libmysqlclient-dev 或检查路径！")
endif()

# ========== 修复：手动指定 OpenSSL 路径 ==========
# 1. 先检查系统默认的 OpenSSL 路径（Ubuntu/Debian 通用）
set(OPENSSL_ROOT_DIR "/usr/lib/x86_64-linux-gnu")
set(OPENSSL_INCLUDE_DIR "/usr/include/openssl")
# 手动指定加密库（解决 OPENSSL_CRYPTO_LIBRARY 缺失问题）
set(OPENSSL_CRYPTO_LIBRARY "/usr/lib/x86_64-linux-gnu/libcrypto.so")
set(OPENSSL_SSL_LIBRARY "/usr/lib/x86_64-linux-gnu/libssl.so")

# 验证 OpenSSL 库是否存在
if(NOT EXISTS ${OPENSSL_CRYPTO_LIBRARY} OR NOT EXISTS ${OPENSSL_INCLUDE_DIR})
    message(FATAL_ERROR "未找到 OpenSSL 库！请先执行：sudo apt install libssl-dev")
endif()

# ========== 包含头文件目录 ==========
include_directories(
        ${MYSQL_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
)

# ========== 编译服务端 ==========
add_executable(MultiMediaToolServer main.cpp)

# 链接库（MySQL + OpenSSL + 线程库）
target_link_libraries(MultiMediaToolServer
        ${MYSQL_LIB}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${OPENSSL_SSL_LIBRARY}
        pthread
)

# 可选：设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)