# MultiMediaTool 服务器集成指南

## 概述

本文档介绍了如何部署和运行MultiMediaTool服务器，以及客户端与服务器之间的交互。

## 服务器组件

### 1. 数据库设置

**MySQL数据库表结构：**

```sql
-- 运行以下SQL文件创建数据库和表
mysql -u root -p < server/mysql_schema.sql
```

主要表：
- `users` - 用户信息
- `ffmpeg_commands` - FFmpeg命令存储
- `user_sessions` - 用户会话管理
- `login_logs` - 登录日志

### 2. 服务器编译和部署

#### 依赖安装

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install libssl-dev libmysqlclient-dev
sudo apt install libboost-all-dev
sudo apt install libcpprest-dev

# 如果没有cpprestsdk，使用vcpkg安装
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install cpprestsdk
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake openssl-devel mysql-devel boost-devel
# cpprestsdk需要从源码编译或使用vcpkg
```

**Windows:**
```powershell
# 使用vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
.\vcpkg install cpprestsdk openssl mysql boost
```

#### 编译服务器

```bash
cd server
chmod +x build.sh
./build.sh

# 或手动编译
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

#### 运行服务器

```bash
# 基本运行
./build/MultiMediaToolServer

# 指定参数
./MultiMediaToolServer \
    --host 0.0.0.0 \
    --port 8080 \
    --db-host localhost \
    --db-user root \
    --db-password password \
    --db-name multimediatool \
    --db-port 3306
```

### 3. API接口

#### 认证接口

**用户注册:**
```
POST /api/auth/register
Content-Type: application/json

{
    "username": "testuser",
    "email": "test@example.com",
    "password": "hashed_password",
    "role": "user"
}
```

**用户登录:**
```
POST /api/auth/login
Content-Type: application/json

{
    "username": "testuser",
    "password": "hashed_password"
}

Response:
{
    "success": true,
    "data": {
        "token": "jwt_token",
        "user": {
            "id": 1,
            "username": "testuser",
            "role": "user"
        }
    }
}
```

**Token验证:**
```
GET /api/auth/validate
Authorization: Bearer jwt_token
```

#### FFmpeg命令接口

**保存命令:**
```
POST /api/commands
Authorization: Bearer jwt_token
Content-Type: application/json

{
    "commandName": "video_convert",
    "inputFile": "/path/to/input.mp4",
    "outputFile": "/path/to/output.mp4",
    "commandLine": "ffmpeg -i input.mp4 -c:v libx264 output.mp4",
    "parameters": "{\"resolution\": \"1920x1080\"}"
}
```

**获取用户命令:**
```
GET /api/commands/user?limit=50&offset=0
Authorization: Bearer jwt_token
```

## 客户端集成

### 1. 登录界面集成

在主应用程序中集成登录界面：

```cpp
#include "gui/page/login.h"
#include "gui/MainWindow.h"

// 在MainWindow中
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    
    // 创建登录页面
    loginPage = new login(this);
    stackedWidget->addWidget(loginPage);
    
    // 连接信号
    connect(loginPage, &login::loginSuccess, 
            this, &MainWindow::onLoginSuccess);
    
    // 默认显示登录页面
    showLoginPage();
}
```

### 2. API管理器使用

```cpp
#include "core/ApiManager.h"

// 初始化API管理器
auto& api = ApiManager::getInstance();
api.setServerUrl("http://localhost:8080");

// 连接信号
connect(&api, &ApiManager::loginSuccess, 
        this, &MainWindow::onApiLoginSuccess);
connect(&api, &ApiManager::networkError, 
        this, &MainWindow::onNetworkError);

// 执行登录
api.login("username", "password");

// 保存FFmpeg命令
FFmpegCommand cmd;
cmd.commandName = "convert";
cmd.inputFile = "input.mp4";
cmd.outputFile = "output.mp4";
cmd.commandLine = "ffmpeg -i input.mp4 output.mp4";
api.saveCommand(cmd);
```

### 3. 密码加密

客户端密码使用SHA256哈希：

```cpp
QString hashPassword(const QString& password, const QString& salt) {
    QString combined = password + salt;
    QByteArray hash = QCryptographicHash::hash(
        combined.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}
```

## 安全考虑

### 1. OpenSSL加密

服务器使用OpenSSL进行：
- 密码哈希（PBKDF2）
- 会话token生成
- 数据加密（AES）
- HMAC签名

### 2. 数据库安全

- 使用预处理语句防止SQL注入
- 密码使用salt+hash存储
- 定期清理过期会话
- 记录登录日志

### 3. 网络安全

- HTTPS传输（生产环境）
- JWT token认证
- CORS配置
- 请求频率限制

## 部署配置

### 1. 生产环境配置

```bash
# 环境变量
export DB_HOST="your-db-host"
export DB_USER="your-db-user"
export DB_PASSWORD="your-db-password"
export DB_NAME="multimediatool"
export JWT_SECRET="your-jwt-secret-key"
export SERVER_PORT=8080
export SERVER_HOST="0.0.0.0"
```

### 2. Nginx配置

```nginx
server {
    listen 80;
    server_name your-domain.com;
    
    location /api/ {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    }
}
```

### 3. SSL证书

```bash
# 使用Let's Encrypt
sudo certbot --nginx -d your-domain.com
```

## 故障排除

### 1. 常见问题

**数据库连接失败:**
- 检查MySQL服务状态
- 验证连接参数
- 检查防火墙设置

**编译错误:**
- 确保所有依赖已安装
- 检查CMake版本
- 验证库路径

**网络连接问题:**
- 检查服务器地址和端口
- 验证防火墙配置
- 检查客户端网络设置

### 2. 日志查看

```bash
# 服务器日志
./MultiMediaToolServer > server.log 2>&1

# MySQL日志
sudo tail -f /var/log/mysql/error.log

# 系统日志
journalctl -u multimediatool
```

## 性能优化

### 1. 数据库优化

- 添加适当的索引
- 配置连接池
- 定期清理日志

### 2. 服务器优化

- 使用反向代理
- 启用Gzip压缩
- 配置缓存

### 3. 客户端优化

- 异步请求处理
- 本地缓存
- 错误重试机制

## 测试

### 1. 单元测试

```bash
cd server
mkdir test && cd test
# 编写单元测试
make test
```

### 2. API测试

使用Postman或curl测试API：

```bash
# 测试登录
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"test","password":"password"}'
```

## 许可证

本项目使用MIT许可证。详见LICENSE文件。