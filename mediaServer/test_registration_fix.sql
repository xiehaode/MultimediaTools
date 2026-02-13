-- 测试注册功能修复的SQL脚本

-- 1. 清理测试数据（如果存在）
DELETE FROM users WHERE username LIKE 'test_%' OR email LIKE 'test_%';

-- 2. 测试插入用户数据（模拟注册）
INSERT INTO users (username, email, password_hash, salt, role) 
VALUES (
    'test_user_123',
    'test@example.com', 
    'hashed_password_here',
    'test_salt_value_here',
    'user'
);

-- 3. 验证插入是否成功
SELECT 
    id,
    username,
    email,
    HEX(salt) as salt_hex,
    LENGTH(salt) as salt_length,
    character_set_name,
    collation_name
FROM information_schema.columns 
WHERE table_schema = 'multimediatool' 
  AND table_name = 'users' 
  AND column_name = 'salt';

-- 4. 查看插入的用户数据
SELECT id, username, email, salt, created_at 
FROM users 
WHERE username = 'test_user_123';

-- 5. 清理测试数据
-- DELETE FROM users WHERE username = 'test_user_123';