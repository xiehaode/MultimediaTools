#include "HttpServer.h"
#include "DatabaseManager.h"
#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>

// 全局服务器实例，用于信号处理
HttpServer* g_server = nullptr;

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // 默认配置
    std::string dbHost = "localhost";
    std::string dbUser = "root";
    std::string dbPassword = "123456";
    std::string dbName = "multimediatool";
    int dbPort = 3306;
    
    std::string serverHost = "0.0.0.0";
    int serverPort = 8080;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--db-host" && i + 1 < argc) {
            dbHost = argv[++i];
        } else if (arg == "--db-user" && i + 1 < argc) {
            dbUser = argv[++i];
        } else if (arg == "--db-password" && i + 1 < argc) {
            dbPassword = argv[++i];
        } else if (arg == "--db-name" && i + 1 < argc) {
            dbName = argv[++i];
        } else if (arg == "--db-port" && i + 1 < argc) {
            dbPort = std::atoi(argv[++i]);
        } else if (arg == "--host" && i + 1 < argc) {
            serverHost = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            serverPort = std::atoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "MultiMediaTool Server\n"
                      << "Usage: " << argv[0] << " [options]\n\n"
                      << "Database Options:\n"
                      << "  --db-host <host>     Database host (default: localhost)\n"
                      << "  --db-user <user>     Database user (default: root)\n"
                      << "  --db-password <pass> Database password (default: password)\n"
                      << "  --db-name <name>     Database name (default: multimediatool)\n"
                      << "  --db-port <port>     Database port (default: 3306)\n\n"
                      << "Server Options:\n"
                      << "  --host <host>        Server bind host (default: 0.0.0.0)\n"
                      << "  --port <port>        Server port (default: 8080)\n\n"
                      << "Other:\n"
                      << "  --help               Show this help message\n";
            return 0;
        }
    }
    
    std::cout << "=== MultiMediaTool Server ===" << std::endl;
    std::cout << "Database: " << dbUser << "@" << dbHost << ":" << dbPort << "/" << dbName << std::endl;
    std::cout << "Server: " << serverHost << ":" << serverPort << std::endl << std::endl;
    
    try {
        // 初始化数据库连接
        auto& db = DatabaseManager::getInstance();
        if (!db.connect(dbHost, dbUser, dbPassword, dbName, dbPort)) {
            std::cerr << "Failed to connect to database!" << std::endl;
            return 1;
        }
        std::cout << "Database connected successfully!" << std::endl;
        
        // 初始化HTTP服务器
        auto& server = HttpServer::getInstance();
        g_server = &server;
        
        // 设置JWT密钥（实际生产环境应该从配置文件或环境变量读取）
        server.setJwtSecret("your-secret-key-change-this-in-production");
        server.setTokenExpirationHours(24);
        
        // 启动HTTP服务器
        if (!server.start(serverHost, serverPort)) {
            std::cerr << "Failed to start HTTP server!" << std::endl;
            return 1;
        }
        std::cout << "HTTP server started successfully!" << std::endl;
        std::cout << "API Endpoints:" << std::endl;
        std::cout << "  POST /api/auth/register - User registration" << std::endl;
        std::cout << "  POST /api/auth/login - User login" << std::endl;
        std::cout << "  GET  /api/auth/validate - Token validation" << std::endl;
        std::cout << "  GET  /api/user/profile - Get user profile" << std::endl;
        std::cout << "  POST /api/commands - Save FFmpeg command" << std::endl;
        std::cout << "  PUT  /api/commands - Update command status" << std::endl;
        std::cout << "  GET  /api/commands/user - Get user commands" << std::endl;
        std::cout << std::endl;
        
        // 保持服务器运行
        std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;
        
        // 等待信号或手动停止
        while (server.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Server stopped successfully." << std::endl;
    return 0;
}