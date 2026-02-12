#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include<openssl/ssl.h>
#include<openssl/err.h>

// MySQL 配置信息
#define MYSQL_HOST "localhost"
#define MYSQL_USER "root"
#define MYSQL_PASS "root"
#define MYSQL_DB   "login_system"

// 网络配置
#define PORT 8888
#define BUF_SIZE 1024

// 验证账号密码函数
bool verify_login(const char* username, const char* password) {
    MYSQL* mysql = mysql_init(nullptr);
    if (!mysql) {
        std::cerr << "MySQL 初始化失败: " << mysql_error(mysql) << std::endl;
        return false;
    }

    // 连接数据库
    if (!mysql_real_connect(mysql, MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DB, 0, nullptr, 0)) {
        std::cerr << "MySQL 连接失败: " << mysql_error(mysql) << std::endl;
        mysql_close(mysql);
        return false;
    }

    // 拼接查询语句（注意：实际项目中需用预处理语句防止SQL注入）
    char query[BUF_SIZE];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username='%s' AND password='%s'", username, password);

    // 执行查询
    if (mysql_query(mysql, query)) {
        std::cerr << "查询失败: " << mysql_error(mysql) << std::endl;
        mysql_close(mysql);
        return false;
    }

    // 获取查询结果
    MYSQL_RES* res = mysql_store_result(mysql);
    bool success = (mysql_num_rows(res) > 0);

    // 释放资源
    mysql_free_result(res);
    mysql_close(mysql);

    return success;
}

int main() {
    // 1. 创建服务端套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "创建套接字失败" << std::endl;
        return -1;
    }

    // 2. 设置套接字选项（避免端口占用问题）
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // 3. 绑定地址和端口
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // 监听所有网卡
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "绑定失败" << std::endl;
        close(server_fd);
        return -1;
    }

    // 4. 监听连接
    if (listen(server_fd, 3) < 0) {
        std::cerr << "监听失败" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "服务端启动成功，监听端口 " << PORT << "..." << std::endl;

    while (true) {
        // 5. 接受客户端连接
        int addr_len = sizeof(address);
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addr_len);
        if (new_socket < 0) {
            std::cerr << "接受连接失败" << std::endl;
            continue;
        }

        std::cout << "客户端 " << inet_ntoa(address.sin_addr) << " 已连接" << std::endl;

        // 6. 接收客户端发送的账号密码（格式：username:password）
        char buffer[BUF_SIZE] = {0};
        ssize_t valread = read(new_socket, buffer, BUF_SIZE);
        if (valread <= 0) {
            std::cerr << "读取数据失败" << std::endl;
            close(new_socket);
            continue;
        }

        // 解析账号和密码
        char* username = strtok(buffer, ":");
        char* password = strtok(nullptr, ":");
        if (!username || !password) {
            const char* err_msg = "格式错误！正确格式：username:password";
            send(new_socket, err_msg, strlen(err_msg), 0);
            close(new_socket);
            continue;
        }

        std::cout << "验证登录：账号=" << username << "，密码=" << password << std::endl;

        // 7. 验证账号密码
        bool login_success = verify_login(username, password);
        const char* response = login_success ? "登录成功！" : "账号或密码错误！";

        // 8. 发送验证结果给客户端
        send(new_socket, response, strlen(response), 0);
        std::cout << "发送结果：" << response << std::endl;

        // 9. 关闭客户端连接
        close(new_socket);
    }

    // 关闭服务端套接字（实际不会执行到这里）
    close(server_fd);
    return 0;
}