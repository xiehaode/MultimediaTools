-- 快速修复脚本 - 仅修复字符编码问题
-- 在现有数据库基础上执行

-- 设置默认字符集
SET NAMES utf8mb4;
SET CHARACTER SET utf8mb4;

-- 修改数据库字符集
ALTER DATABASE multimediatool 
CHARACTER SET = utf8mb4 
COLLATE = utf8mb4_unicode_ci;

-- 修改表字符集
ALTER TABLE users 
CONVERT TO CHARACTER SET utf8mb4 
COLLATE utf8mb4_unicode_ci;

ALTER TABLE ffmpeg_commands 
CONVERT TO CHARACTER SET utf8mb4 
COLLATE utf8mb4_unicode_ci;

-- 验证修复
SELECT 
    table_schema,
    table_name,
    table_collation
FROM information_schema.tables 
WHERE table_schema = 'multimediatool';

SELECT '字符编码修复完成!' as result;