-- 修复数据库字符编码问题的脚本
-- 解决 salt 列的字符编码错误

-- 1. 备份现有数据（如果需要）
-- CREATE TABLE users_backup AS SELECT * FROM users;

-- 2. 修改数据库字符集
ALTER DATABASE multimediatool 
CHARACTER SET = utf8mb4 
COLLATE = utf8mb4_unicode_ci;

-- 3. 修改表字符集
ALTER TABLE users 
CONVERT TO CHARACTER SET utf8mb4 
COLLATE utf8mb4_unicode_ci;

-- 4. 修改特定列的字符集（如果仍有问题）
-- 注意：某些MySQL版本可能不支持在MODIFY COLUMN中直接设置CHARACTER SET
-- 如果失败，请使用下面的ALTER TABLE CONVERT方法

-- 5. 验证字符集设置
SELECT 
    table_schema,
    table_name,
    table_collation
FROM 
    information_schema.tables 
WHERE 
    table_schema = 'multimediatool' AND table_name = 'users';

SELECT 
    column_name,
    data_type,
    character_set_name,
    collation_name
FROM 
    information_schema.columns 
WHERE 
    table_schema = 'multimediatool' AND table_name = 'users' 
    AND column_name IN ('username', 'email', 'password_hash', 'salt');

-- 执行完成后，可以重启应用程序测试注册功能