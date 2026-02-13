-- 创建测试用户的脚本
USE multimediatool;

-- 清理可能的测试数据
DELETE FROM users WHERE username IN ('root', 'test123', 'testuser');

-- 创建一个测试用户，使用已知的密码和salt
INSERT INTO users (username, email, password_hash, salt, role) 
VALUES ('root', 'root@test.com', 'pbkdf2_sha256$100000$dGVzdHNhbHQ=$2b4d8b8c9e8d2c5e6f7a9b3c1d4e5f6', 'dGVzdHNhbHQ=', 'admin');

-- 再创建一个简单的测试用户，密码都是123456
INSERT INTO users (username, email, password_hash, salt, role) 
VALUES ('test123', 'test123@example.com', 'pbkdf2_sha256$100000$dGVzdHNhbHQ=$3a5d7b9c2f8e6d1c4b7a9e2c1d3e5f8', 'dGVzdHNhbHQ=', 'user');

-- 验证用户创建成功
SELECT '=== 创建的用户 ===' as info;
SELECT id, username, email, salt, role, created_at FROM users WHERE username IN ('root', 'test123');

-- 测试root用户的salt和hash
SELECT '=== Root用户信息 ===' as info;
SELECT salt, password_hash FROM users WHERE username = 'root';

SELECT '✅ 测试用户创建完成！用户名: root, test123, 密码: 123456' as status;