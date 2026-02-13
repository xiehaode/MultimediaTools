# 注册功能修复指南

## 问题解决

### 1. 数据库字符编码问题 ❌➜✅

**错误**: `Incorrect string value: "x86\xA8\xC1x14MxE2..' for column'p_salt' at row 1`

**原因**: MySQL默认字符集不支持二进制数据存储

**解决方案**:

#### 方法1: 更新现有数据库
```bash
mysql -u root -p multimediatool < mediaServer/fix_encoding.sql
```

#### 方法2: 重新创建数据库
```bash
mysql -u root -p < mediaServer/mysql_schema.sql
```

**关键修改**:
- 数据库字符集: `utf8mb4_unicode_ci`
- 表字符集: `utf8mb4_unicode_ci`  
- 相关列字符集: `utf8mb4_unicode_ci`

### 2. SSL/TLS连接问题 ❌➜✅

**错误**: `QSslSocket::connectToHostEncrypted: TLS initialization failed`

**原因**: 开发环境中SSL证书配置问题

**解决方案**: 已在代码中配置SSL忽略（仅用于开发测试）

**代码修改**:
```cpp
// 在构造函数中配置全局SSL设置
QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
QSslConfiguration::setDefaultConfiguration(sslConfig);

// 为每个网络请求配置SSL忽略
void FfmpegClientWidget::configureNetworkRequest(QNetworkRequest &request)
{
    QSslConfiguration sslConfig = request.sslConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(sslConfig);
}
```

## 使用步骤

### 1. 修复数据库

**方法A: 快速修复（推荐）**
```bash
mysql -u root -p multimediatool < mediaServer/quick_fix.sql
```

**方法B: 完全重建**
```bash
mysql -u root -p < mediaServer/create_database_simple.sql
```

**方法C: 原脚本修复**
```bash
mysql -u root -p multimediatool < mediaServer/fix_encoding.sql
```

**验证修复**:
```bash
mysql -u root -p -e "USE multimediatool; SHOW CREATE TABLE users;"
```

### 2. 重新编译客户端
```bash
cd d:/vsPro/MultiMediaTool/MultiMediaTool
qmake && nmake clean && nmake
```

### 3. 测试注册功能
1. 启动应用
2. 点击"注册"按钮
3. 填写注册表单
4. 提交注册

## API端点

### 注册
```
POST http://localhost:8080/api/auth/register
Content-Type: application/json

{
    "username": "testuser",
    "password": "password123", 
    "email": "test@example.com"
}
```

### 响应格式
**成功**:
```json
{
    "success": true,
    "message": "用户注册成功",
    "user_id": 123
}
```

**失败**:
```json
{
    "success": false,
    "error": "用户名已存在"
}
```

## 验证测试

### 数据库验证
```sql
-- 测试脚本
mysql -u root -p multimediatool < mediaServer/test_registration_fix.sql
```

### 功能验证清单
- [ ] 注册页面正常显示
- [ ] 表单验证工作正常
- [ ] 注册请求成功发送
- [ ] 服务器正确处理注册
- [ ] 数据成功存入数据库
- [ ] 注册成功后跳转登录页面

## 安全注意事项

⚠️ **生产环境配置**:
1. 启用HTTPS和SSL验证
2. 移除SSL忽略代码
3. 配置有效的SSL证书
4. 加强密码强度要求
5. 添加邮箱验证机制

## 故障排除

### 如果仍有字符编码问题
```sql
-- 检查当前字符集设置
SHOW VARIABLES LIKE 'character_set%';
SHOW VARIABLES LIKE 'collation%';

-- 重新设置会话字符集
SET NAMES utf8mb4;
SET CHARACTER SET utf8mb4;
```

### 如果仍有SSL问题
1. 检查服务器是否支持HTTPS
2. 确认使用HTTP而非HTTPS进行测试
3. 检查防火墙和代理设置

## 文件变更摘要

**修改的文件**:
- `mediaServer/mysql_schema.sql` - 添加UTF8MB4字符集
- `ffmpegclientwidget.h` - 添加SSL配置函数声明
- `ffmpegclientwidget.cpp` - 实现SSL忽略和注册功能

**新增的文件**:
- `mediaServer/fix_encoding.sql` - 数据库修复脚本
- `mediaServer/test_registration_fix.sql` - 测试脚本
- `REGISTRATION_FIX_GUIDE.md` - 本指南

注册功能现在应该可以正常工作了！🎉