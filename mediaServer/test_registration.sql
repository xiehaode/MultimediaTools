-- 测试用户注册功能的SQL脚本
USE multimediatool;

-- 清理测试数据
DELETE FROM users WHERE username = 'testuser' OR email = 'test@example.com';

-- 测试存储过程调用
CALL RegisterUser('testuser', 'test@example.com', 'test_hash', 'dGVzdHNhbHQ=', 'user', @user_id, @status, @message);

-- 查看结果
SELECT @user_id as user_id, @status as status, @message as message;

-- 验证用户是否成功创建
SELECT id, username, email, salt, role, created_at FROM users WHERE username = 'testuser';