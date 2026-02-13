# MultiMediaTool Server 部署指南

## 项目概述

MultiMediaTool Server 是一个基于 C++ 的多媒体处理服务器，提供以下功能：

- 用户认证和授权系统
- FFmpeg 命令执行和管理
- 文件上传和下载
- 命令历史记录和状态跟踪
- 实时进度监控

## 系统要求

### 硬件要求
- CPU: 2核或以上
- 内存: 2GB或以上
- 磁盘空间: 10GB或以上

### 软件要求
- 操作系统: Ubuntu 20.04 LTS 或更高版本
- MySQL: 8.0.45 或更高版本
- CMake: 3.16 或更高版本
- GCC: 9.0 或更高版本
- OpenSSL: 1.1.1 或更高版本
- Boost: 1.71 或更高版本

## 安装依赖

### 基础依赖

```bash
sudo apt update
sudo apt install -y build-essential cmake git pkg-config
sudo apt install -y libssl-dev libmysqlclient-dev libboost-all-dev
```

### MySQL 安装

```bash
# 安装 MySQL 8.0
sudo apt install -y mysql-server mysql-client

# 启动 MySQL 服务
sudo systemctl start mysql
sudo systemctl enable mysql

# 运行安全配置向导（可选）
sudo mysql_secure_installation
```

## 数据库设置

### 1. 创建数据库和用户

```bash
# 登录 MySQL
sudo mysql

# 创建数据库
CREATE DATABASE IF NOT EXISTS multimediatool 
CHARACTER SET utf8mb4 
COLLATE utf8mb4_unicode_ci;

# 创建用户并授权
CREATE USER IF NOT EXISTS 'multimediatool'@'localhost' IDENTIFIED BY 'your_password';
GRANT ALL PRIVILEGES ON multimediatool.* TO 'multimediatool'@'localhost';
FLUSH PRIVILEGES;

# 退出 MySQL
EXIT;
```

### 2. 导入数据库结构

```bash
# 使用融合后的 SQL 脚本
mysql -u multimediatool -p multimediatool < mysql_schema.sql

# 输入密码: your_password
```

### 3. 验证数据库设置

```bash
# 登录 MySQL
mysql -u multimediatool -p

# 验证数据库存在
SHOW DATABASES;

# 验证表结构
USE multimediatool;
SHOW TABLES;

# 验证默认管理员用户
SELECT * FROM users WHERE username = 'admin';

# 退出 MySQL
EXIT;
```

## 编译服务器

### 1. 配置 CMake

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置 CMake（完整版本）
cmake .. -DCMAKE_BUILD_TYPE=Release

# 或者配置 CMake（简化版本，减少依赖）
cmake .. -DCMAKE_BUILD_TYPE=Release -DSIMPLE_HTTP=ON
```

### 2. 编译项目

```bash
# 编译
make -j$(nproc)

# 验证编译结果
ls -la MultiMediaToolServer
```

## 运行服务器

### 基本运行

```bash
# 使用默认配置
./MultiMediaToolServer

# 或者使用指定参数
./MultiMediaToolServer \
    --host 0.0.0.0 \
    --port 8080 \
    --db-host localhost \
    --db-user multimediatool \
    --db-password your_password \
    --db-name multimediatool
```

### 运行参数说明

| 参数 | 描述 | 默认值 |
|------|------|--------|
| --host | 服务器监听地址 | 0.0.0.0 |
| --port | 服务器监听端口 | 8080 |
| --db-host | 数据库主机地址 | localhost |
| --db-user | 数据库用户名 | multimediatool |
| --db-password | 数据库密码 | your_password |
| --db-name | 数据库名称 | multimediatool |
| --log-level | 日志级别 | info |
| --config | 配置文件路径 | - |

## API 测试

### 1. 健康检查

```bash
curl http://localhost:8080/api/auth/validate
```

### 2. 用户注册

```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"test","email":"test@example.com","password":"password123"}'
```

### 3. 用户登录

```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"test","password":"password123"}'
```

### 4. 执行 FFmpeg 命令

```bash
# 使用登录返回的 token
TOKEN="your_jwt_token"

curl -X POST http://localhost:8080/api/commands \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer $TOKEN" \
  -d '{"commandName":"测试命令","commandLine":"-i input.mp4 output.mp4","inputFile":"input.mp4","outputFile":"output.mp4","parameters":{}}'
```

## 生产环境部署

### 1. 创建系统用户

```bash
sudo useradd -r -s /bin/false multimediatool
sudo mkdir -p /opt/multimediatool
sudo chown -R multimediatool:multimediatool /opt/multimediatool
```

### 2. 部署应用

```bash
# 复制编译后的文件
sudo cp build/MultiMediaToolServer /opt/multimediatool/
sudo cp mysql_schema.sql /opt/multimediatool/
sudo chown multimediatool:multimediatool /opt/multimediatool/MultiMediaToolServer
```

### 3. 创建 systemd 服务

```bash
sudo tee /etc/systemd/system/multimediatool.service > /dev/null <<EOF
[Unit]
Description=MultiMediaTool Server
After=network.target mysql.service

[Service]
Type=simple
User=multimediatool
WorkingDirectory=/opt/multimediatool
ExecStart=/opt/multimediatool/MultiMediaToolServer \
    --host 0.0.0.0 \
    --port 8080 \
    --db-host localhost \
    --db-user multimediatool \
    --db-password your_password \
    --db-name multimediatool
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# 启用并启动服务
sudo systemctl daemon-reload
sudo systemctl enable multimediatool
sudo systemctl start multimediatool

# 检查服务状态
sudo systemctl status multimediatool
```

### 4. 配置 Nginx 反向代理

```bash
# 安装 Nginx
sudo apt install -y nginx

# 创建配置文件
sudo tee /etc/nginx/sites-available/multimediatool > /dev/null <<EOF
server {
    listen 80;
    server_name your-domain.com;
    
    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
    
    # 处理文件上传
    client_max_body_size 100M;
}
EOF

# 启用配置
sudo ln -s /etc/nginx/sites-available/multimediatool /etc/nginx/sites-enabled/

# 测试配置
sudo nginx -t

# 重启 Nginx
sudo systemctl restart nginx
```

## 故障排除

### 1. 服务启动失败

```bash
# 查看服务日志
sudo journalctl -u multimediatool -f

# 检查数据库连接
mysql -u multimediatool -p multimediatool

# 检查端口占用
netstat -tulpn | grep 8080
```

### 2. 数据库连接问题

- 检查 MySQL 服务状态：`systemctl status mysql`
- 验证数据库用户权限
- 检查防火墙设置
- 确认连接参数正确

### 3. 编译错误

| 错误 | 解决方案 |
|------|----------|
| cmake: command not found | `sudo apt install cmake` |
| MySQL development files not found | `sudo apt install libmysqlclient-dev` |
| OpenSSL development files not found | `sudo apt install libssl-dev` |
| Boost libraries not found | `sudo apt install libboost-all-dev` |
| cpprestsdk not found | `sudo apt install libcpprest-dev` 或使用简化版本 |

### 4. API 调用失败

- 检查服务器日志
- 验证认证 token 是否有效
- 检查请求格式是否正确
- 确认数据库操作权限

## 性能优化

### 1. 数据库优化

```bash
# 启用查询缓存（MySQL 8.0 已移除，使用其他缓存方案）
# 优化表结构
USE multimediatool;
OPTIMIZE TABLE users, ffmpeg_commands, user_sessions, login_logs;
```

### 2. 服务器优化

- 使用多核编译：`make -j$(nproc)`
- 启用编译器优化：`-DCMAKE_BUILD_TYPE=Release`
- 调整系统参数：增加文件描述符限制

### 3. 负载测试

```bash
# 使用 ab 进行压力测试
ab -n 1000 -c 10 http://localhost:8080/api/auth/validate

# 使用 wrk 进行更详细的测试
# 安装 wrk: sudo apt install wrk
wrk -t12 -c400 -d30s http://localhost:8080/api/auth/validate
```

## 安全配置

### 1. 数据库安全

- 使用强密码
- 限制数据库用户权限
- 启用 SSL 连接（可选）

### 2. 服务器安全

- 使用非 root 用户运行服务
- 配置防火墙：`ufw allow 8080/tcp`
- 定期更新依赖
- 启用 HTTPS

### 3. API 安全

- 使用 JWT token 认证
- 实现请求速率限制
- 验证所有输入参数
- 加密敏感数据

## 备份与恢复

### 1. 数据库备份

```bash
# 定期备份
mysqldump -u multimediatool -p multimediatool > multimediatool_backup.sql

# 自动备份脚本
crontab -e
# 添加以下行（每天凌晨 2 点备份）
0 2 * * * mysqldump -u multimediatool -p'your_password' multimediatool > /backup/multimediatool_$(date +\%Y\%m\%d).sql
```

### 2. 恢复备份

```bash
# 恢复数据库
mysql -u multimediatool -p multimediatool < multimediatool_backup.sql
```

## 升级指南

### 1. 停止服务

```bash
sudo systemctl stop multimediatool
```

### 2. 备份数据

```bash
mysqldump -u multimediatool -p multimediatool > multimediatool_backup.sql
```

### 3. 拉取最新代码

```bash
git pull origin master
```

### 4. 重新编译

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 5. 更新数据库

```bash
mysql -u multimediatool -p multimediatool < mysql_schema.sql
```

### 6. 重启服务

```bash
sudo cp MultiMediaToolServer /opt/multimediatool/
sudo chown multimediatool:multimediatool /opt/multimediatool/MultiMediaToolServer
sudo systemctl start multimediatool
```

## 贡献指南

1. Fork 项目
2. 创建功能分支：`git checkout -b feature/new-feature`
3. 提交更改：`git commit -am 'Add new feature'`
4. 推送分支：`git push origin feature/new-feature`
5. 创建 Pull Request

## 许可证

MIT License - 详见 LICENSE 文件

## 联系信息

- 项目地址：https://github.com/yourusername/MultiMediaTool
- 问题反馈：https://github.com/yourusername/MultiMediaTool/issues

---

**注意**：本部署指南适用于 Ubuntu 20.04 LTS 及更高版本。对于其他操作系统，请根据实际情况调整命令和配置。