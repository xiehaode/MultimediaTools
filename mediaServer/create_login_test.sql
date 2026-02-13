-- 创建登录测试用户 - 使用简单哈希验证
USE multimediatool;

-- 清理现有用户
DELETE FROM users;

-- 创建测试用户，密码都是123456
-- 使用 sha256("123456root") 作为哈希值
INSERT INTO users (username, email, password_hash, salt, role, is_active) 
VALUES 
('root', 'root@test.com', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'salt123', 'admin', 1),
('test', 'test@test.com', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'salt456', 'user', 1),
('admin', 'admin@test.com', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'salt789', 'admin', 1);

-- 验证创建的用户
SELECT '=== 登录测试用户 ===' as info;
SELECT id, username, email, LEFT(password_hash, 20) as hash_prefix, role, is_active FROM users;

-- 测试存储过程
CALL RegisterUser('newuser', 'new@test.com', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'newsalt', 'user', @uid, @stat, @msg);
SELECT @uid as user_id, @stat as status, @msg as message;

SELECT '✅ 登录测试数据创建完成！所有用户密码: 123456' as final_status;