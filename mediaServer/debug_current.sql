-- 调试当前用户状态
USE multimediatool;

-- 查看所有用户
SELECT '=== 所有用户 ===' as info;
SELECT id, username, email, salt, LEFT(password_hash, 30) as hash_prefix, role, is_active FROM users ORDER BY id;

-- 测试root用户是否存在
SELECT '=== Root用户检查 ===' as info;
SELECT COUNT(*) as root_count, 
       CASE WHEN COUNT(*) > 0 THEN 'EXISTS' ELSE 'NOT EXISTS' END as root_status
FROM users WHERE username = 'root';

-- 如果root用户不存在，创建一个真实的root用户
INSERT IGNORE INTO users (username, email, password_hash, salt, role) 
VALUES ('root', 'admin@multimedia.com', 'root_password_hash', 'rootsalt123', 'admin');

-- 手动测试密码哈希生成（用于调试）
SELECT '=== 密码哈希测试 ===' as info;

-- 创建一个简化版的root用户，使用简单的密码验证
DELETE FROM users WHERE username = 'root';

-- 创建一个使用简单密码的root用户（密码：123456）
INSERT INTO users (username, email, password_hash, salt, role) 
VALUES ('root', 'admin@multimedia.com', 'sha256_123456_root', 'simple_salt', 'admin');

-- 验证创建
SELECT '=== 创建的root用户 ===' as info;
SELECT id, username, email, salt, password_hash FROM users WHERE username = 'root';

SELECT '✅ 调试脚本完成！现在尝试用 root/123456 登录' as status;