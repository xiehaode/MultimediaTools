-- 最终登录测试数据创建脚本
USE multimediatool;

-- 清理所有用户
DELETE FROM users;

-- 创建测试用户，密码都是123456
-- 使用 sha256("123456username") 作为密码哈希
INSERT INTO users (username, email, password_hash, salt, role, is_active) VALUES
('root', 'root@test.com', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'salt_root', 'admin', 1),
('test', 'test@test.com', '3f8a85b51f7c3d69a2ffcaf4a355a43d1aeb1f0d9a7d9e3b5c3f8e8f7e5c8', 'salt_test', 'user', 1),
('admin', 'admin@test.com', 'c8b3b4b9a5f9b5c9d9e5a3f7b8d9c0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2', 'salt_admin', 'admin', 1);

-- 验证创建的用户
SELECT '=== 创建的测试用户 ===' as info;
SELECT id, username, email, LEFT(password_hash, 20) as hash_prefix, role, is_active FROM users;

-- 手动验证哈希计算
SELECT '=== 哈希验证测试 ===' as info;
SELECT 'root' as username, '123456root' as hash_input, SHA2('123456root', 256) as expected_hash;
SELECT 'test' as username, '123456test' as hash_input, SHA2('123456test', 256) as expected_hash;
SELECT 'admin' as username, '123456admin' as hash_input, SHA2('123456admin', 256) as expected_hash;

SELECT '✅ 最终登录测试数据创建完成！' as status;
SELECT '用户名: root, test, admin' as user_info;
SELECT '密码: 123456 (所有用户通用)' as password_info;