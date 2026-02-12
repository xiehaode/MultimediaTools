cmake_minimum_required(VERSION 3.10)
project(mediaServer VERSION 1.0.0 LANGUAGES CXX)

# ===================== 基础配置 =====================
# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 配置选项
option(SIMPLE_HTTP "Use simple HTTP implementation instead of cpprestsdk" OFF)
option(ENABLE_TESTING "Build tests" OFF)

# ===================== 依赖查找 =====================
# 1. 基础依赖
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)

# 2. MySQL/MariaDB (简化查找逻辑)
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
        PATHS /usr/include/mysql
        DOC "MySQL include directory"
)
find_library(MYSQL_LIBRARY NAMES mysqlclient mariadbclient
        PATHS /usr/lib/x86_64-linux-gnu
        DOC "MySQL library"
)
set(MYSQL_FOUND FALSE)
if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    set(MYSQL_FOUND TRUE)
    set(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})
    set(MYSQL_LIBRARIES ${MYSQL_LIBRARY})
else()
    message(WARNING "MySQL/MariaDB not found! Install: libmysqlclient-dev (Debian) / mysql-devel (RHEL)")
endif()

# 3. cpprestsdk (简化查找+逻辑)
set(cpprestsdk_FOUND FALSE)
find_path(CPPRESTSDK_INCLUDE_DIR cpprest/http_client.h
        PATHS /usr/include /usr/include/cpprest /usr/local/include
)
find_library(CPPRESTSDK_LIBRARY NAMES cpprest cpprestsdk
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)
if(CPPRESTSDK_INCLUDE_DIR AND CPPRESTSDK_LIBRARY AND NOT SIMPLE_HTTP)
    set(cpprestsdk_FOUND TRUE)
    add_library(cpprestsdk::cpprestsdk SHARED IMPORTED)
    set_target_properties(cpprestsdk::cpprestsdk PROPERTIES
            IMPORTED_LOCATION "${CPPRESTSDK_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${CPPRESTSDK_INCLUDE_DIR}"
    )
endif()

# ===================== 编译配置 =====================
# 包含目录 (统一管理)
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}  # 配置头文件目录
)
if(MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIRS})
endif()
if(cpprestsdk_FOUND)
    include_directories(${CPPRESTSDK_INCLUDE_DIR})
endif()

# 源文件
set(CORE_SOURCES
        crypto/CryptoUtils.cpp
        DatabaseManager.cpp
        HttpServer.cpp
)
set(MAIN_SOURCES main.cpp)

# 静态库 (合并为核心库，简化结构)
add_library(server_core STATIC ${CORE_SOURCES})
# 静态库不需要SOVERSION，仅保留VERSION
set_target_properties(server_core PROPERTIES
        VERSION ${PROJECT_VERSION}
)

# 链接库 (按条件链接)
target_link_libraries(server_core
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
)
if(MYSQL_FOUND)
    target_link_libraries(server_core ${MYSQL_LIBRARIES})
endif()
if(cpprestsdk_FOUND)
    target_link_libraries(server_core cpprestsdk::cpprestsdk)
    target_compile_definitions(server_core PRIVATE WITH_CPPRESTSDK)
else()
    target_compile_definitions(server_core PRIVATE SIMPLE_HTTP)
    # 自动创建简化HTTP实现文件（仅当文件不存在时）
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SimpleHttpServer.cpp)
        file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/SimpleHttpServer.cpp "// Simple HTTP server placeholder\n")
    endif()
endif()

# 可执行文件 (修复：确保主函数正确链接核心库)
add_executable(${PROJECT_NAME} ${MAIN_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
target_link_libraries(${PROJECT_NAME} server_core)

# 线程编译选项 (适配gcc/clang)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # 同时给库和可执行文件加线程选项，避免链接错误
    target_compile_options(server_core PRIVATE -pthread)
    target_compile_options(${PROJECT_NAME} PRIVATE -pthread)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

# ===================== 辅助功能 =====================
# 配置头文件
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

# 安装规则
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)
install(FILES mysql_schema.sql DESTINATION share/${PROJECT_NAME})

# 调试目标 (Debug模式下)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_target(debug-server
            COMMAND gdb ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Start GDB debug session"
    )
    add_custom_target(valgrind-server
            COMMAND valgrind --leak-check=full ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Run Valgrind memory check"
    )
endif()

# 测试支持
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# CPack打包
include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MultiMediaTool HTTP Server")
set(CPACK_PACKAGE_VENDOR "MultiMediaTool Project")

# 简洁的配置信息输出 (修复：改用CMake的if判断替代三元表达式)
message(STATUS "=== Build Config ===")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(cpprestsdk_FOUND)
    message(STATUS "HTTP Backend: cpprestsdk")
else()
    message(STATUS "HTTP Backend: Simple HTTP")
endif()
message(STATUS "MySQL Support: ${MYSQL_FOUND}")