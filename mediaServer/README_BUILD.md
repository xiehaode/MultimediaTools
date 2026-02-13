# MultiMediaTool Server 构建指南

## 快速开始

### 1. 安装依赖

```bash

sudo apt update
sudo apt install -y build-essential cmake git pkg-config
sudo apt install -y libssl-dev libmysqlclient-dev libboost-all-dev
```

### 2. 数据库设置

```bash
# 启动MySQL服务
sudo systemctl start mysql

# 创建数据库
sudo mysql -e "CREATE DATABASE multimediatool;"

# 导入表结构
mysql -u root -p multimediatool < mysql_schema.sql
```

### 3. 编译服务器

CLion
```

## 构建选项

### 完整版本 vs 简化版本

| 特性 | 完整版本 | 简化版本 |
|------|----------|----------|
| HTTP服务器 | cpprestsdk | 简单HTTP实现 |
| JSON处理 | cpprestsdk | 手动实现 |
| 异步处理 | 支持 | 基础支持 |
| 文件上传 | 支持 | 有限支持 |
| 构建依赖 | 更多 | 较少 |

### 常见问题

#### 1. cmake: command not found
```bash
sudo apt install cmake
```

#### 2. MySQL development files not found
```bash
sudo apt install libmysqlclient-dev
```

#### 3. OpenSSL development files not found
```bash
sudo apt install libssl-dev
```

#### 4. Boost libraries not found
```bash
sudo apt install libboost-all-dev
```

#### 5. cpprestsdk not found
```bash
# Ubuntu/Debian
sudo apt install libcpprest-dev

# 或使用简化版本
./simple_build.sh
```

## 运行服务器

### 基本运行

```bash
# 使用默认配置
./build/MultiMediaToolServer

# 指定参数
./build/MultiMediaToolServer \
    --host 0.0.0.0 \
    --port 8080 \
    --db-host localhost \
    --db-user root \
    --db-password your_password \
    --db-name multimediatool
```

### 配置文件（未来版本）

```bash
# 使用配置文件
./MultiMediaToolServer --config server.conf
```

## API测试

### 健康检查

```bash
curl http://localhost:8080/api/auth/validate
```

### 用户注册

```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"test","email":"test@example.com","password":"password123"}'
```

### 用户登录

```bash
curl -X POST http://localhost:8080/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"test","password":"password123"}'
```

## 开发环境

### IDE配置

#### VSCode

安装扩展：
- C/C++ Extension Pack
- CMake Tools

#### CLion

导入CMake项目，选择build目录。

### 调试

```bash
# 编译调试版本
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSIMPLE_HTTP=ON
make

# 使用gdb调试
gdb ./MultiMediaToolServer
```

## 生产环境部署

### 1. 编译生产版本

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 2. 创建systemd服务

```bash
sudo tee /etc/systemd/system/multimediatool.service > /dev/null <<EOF
[Unit]
Description=MultiMediaTool Server
After=network.target mysql.service

[Service]
Type=simple
User=multimediatool
WorkingDirectory=/opt/multimediatool
ExecStart=/opt/multimediatool/MultiMediaToolServer
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl enable multimediatool
sudo systemctl start multimediatool
```

### 3. Nginx反向代理

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

## 故障排除

### 日志查看

```bash
# 系统日志
journalctl -u multimediatool -f

# 应用日志
./MultiMediaToolServer --log-level debug
```

### 性能测试

```bash
# 使用ab进行压力测试
ab -n 1000 -c 10 http://localhost:8080/api/auth/validate
```

### 数据库连接问题

1. 检查MySQL服务状态：`systemctl status mysql`
2. 检查防火墙设置
3. 验证用户权限：`mysql -u username -p -e "SHOW DATABASES;"`
4. 检查配置文件中的连接参数

## 贡献指南

1. Fork项目
2. 创建功能分支：`git checkout -b feature/new-feature`
3. 提交更改：`git commit -am 'Add new feature'`
4. 推送分支：`git push origin feature/new-feature`
5. 创建Pull Request

## 许可证

MIT License - 详见LICENSE文件