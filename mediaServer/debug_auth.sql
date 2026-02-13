-- 调试认证问题的SQL脚本
USE multimediatool;

-- 1. 查看当前所有用户
SELECT '=== 当前用户列表 ===' as info;
SELECT id, username, email, LEFT(password_hash, 20) as hash_prefix, 
       LEFT(salt, 20) as salt_prefix, role, is_active, last_login 
FROM users ORDER BY id;

-- 2. 查看是否有root用户
SELECT '=== Root用户检查 ===' as info;
SELECT COUNT(*) as root_count FROM users WHERE username = 'root';

-- 3. 如果没有root用户，创建一个测试用户
INSERT IGNORE INTO users (username, email, password_hash, salt, role) 
VALUES ('root', 'admin@test.com', 'root_test_hash_123456', 'dGVzdHNhbHRyb290', 'admin');

-- 4. 测试认证存储过程
SELECT '=== 测试认证存储过程 ===' as info;
CALL AuthenticateUser('root', 'root_test_hash_123456', @uid, @role, @status, @msg);

SELECT @uid as user_id, @role as role, @status as status, @msg as message;

-- 5. 如果认证成功，更新密码
SELECT '=== 如果需要，更新测试密码 ===' as info;
UPDATE users 
SET password_hash = 'pbkdf2_sha256$100000$root_salt$hash_result_here', 
    salt = 'dGVzdHNhbHRyb290'
WHERE username = 'root';

-- 6. 创建一个已知的测试用户（用于验证）
DELETE FROM users WHERE username = 'test123';
INSERT INTO users (username, email, password_hash, salt, role) 
VALUES ('test123', 'test123@example.com', 'test_hash_789', 'dGVzdHNhbHQ=', 'user');

-- 7. 测试test123用户
CALL AuthenticateUser('test123', 'test_hash_789', @uid2, @role2, @status2, @msg2);
SELECT @uid2 as test_user_id, @role2 as test_role, @status2 as test_status, @msg2 as test_message;

SELECT '✅ 认证调试脚本执行完成！' as final_status;